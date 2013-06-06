/*--------------------------------------------------------------------------------------------------
 "HENSEL MODEL01"
 Arduino "MIDIsync" - MIDI Master Clock Generator
 https://github.com/gretel/MIDIsync
 ɔ 2012-2013 Tom Hensel <tom@interpol8.net> Hamburg, Germany
 CC BY-SA 3.0 http://creativecommons.org/licenses/by-sa/3.0/
 --------------------------------------------------------------------------------------------------*/

// id
#define ID "MIDI_MASTER_CLOCK"
#define VERSION 06062013
#define DEBUG 0

// includes
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include "Arduino.h"

// 3rd party includes
#include <digitalWriteFast.h>
#include <Streaming.h>
#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <movingAvg.h>
#include <BiColorLED.h>
#include <Button.h>
#define ENCODER_DO_NOT_USE_INTERRUPTS
#include <Encoder.h>

#if DEBUG
#include <SoftwareSerial.h>
#define DEBUG_RX 11
#define DEBUG_TX 12
#define DEBUG_SPEED 38400
#define DEBUG_INTERVAL 333
#endif

// hardware
#define BOARD_LED 13
#define MIDI_PORT Serial
#define BTN_ENCODER 2
#define BTN_STATE 4
#define GATE_PIN 8
#define LED_A 9
#define LED_B 10
#define LED_C 5
#define LED_D 6
#define ENC_A 14
#define ENC_B 15
#define ENC_PORT PINC

// states
#define CLOCK_INTERNAL 1
#define CLOCK_EXTERNAL 2
#define CLOCK_SYNC     3

// timing
#define CLOCK_DETECTION_WINDOW 600000 // TODO description
#define HOLD_THRESH 500 // TODO description

struct config_t
{
    uint16_t version;
    uint32_t cycle;
    uint8_t state;
    uint8_t cpqn;
} config;

// static
const uint8_t STATUS_SYNC     = 0xF8;
const uint8_t STATUS_START    = 0xFA;
const uint8_t STATUS_CONTINUE = 0xFB;
const uint8_t STATUS_STOP     = 0xFC;

// var
uint8_t mode; // TODO struct
uint8_t intState; // TODO struct
uint8_t nextIntState; // TODO struct
uint8_t extState; // TODO struct
uint32_t cycleTime;
uint32_t cycleTimeExt;
uint8_t colorRight;
uint8_t colorLeft;
register uint32_t intTime asm("r2");
register uint32_t extTime asm("r6");
register uint8_t cpqnInt asm("r10");
register uint8_t cpqnExt asm("r11");

movingAvg tapTimeFilter;
movingAvg cycleTimeExtFilter;
Button stateButton = Button(BTN_STATE, BUTTON_PULLDOWN, true, 50);
Button tempoButton = Button(BTN_ENCODER, BUTTON_PULLDOWN, true, 50);
BiColorLED ledLeft = BiColorLED(LED_A, LED_B);
BiColorLED ledRight = BiColorLED(LED_C, LED_D);

#if DEBUG
SoftwareSerial debugSerial(DEBUG_RX, DEBUG_TX);
movingAvg profilingFilter;
uint32_t debugTime;
uint32_t loopCycle;

void printDiag()
{
    const uint16_t duration = micros() - loopCycle;
    const uint16_t average = profilingFilter.reading(duration);

    debugSerial << " COUNT: " << micros()
                << " LOOP: " << duration
                << " AVG: " << average
                << " MODE: " << mode
                << " CYLC_I: " << cycleTime
                << " CYLC_E: " << cycleTimeExt
                << " STATE_I: " << intState
                << " STATE_E: " << extState
                << " CPQN_I: " << cpqnInt
                << " CPQN_E: " << cpqnExt
                << endl;
}
#endif

void resetConfig()
{
#if DEBUG
    debugSerial << "CONFIG:RESET" << endl;
#endif

    digitalWriteFast(LED_A, HIGH);
    digitalWriteFast(LED_C, HIGH);

    // initialize configuration structure
    config.version = (uint16_t)VERSION;
    config.cpqn = 24;
    config.cycle = 20000;
    config.state = STATUS_STOP;

    // write to non-volatile memory
    EEPROM_writeAnything(0, config);
    do
    {
        delay(100);
    }
    while (stateButton.isPressed() || tempoButton.isPressed());

    digitalWriteFast(LED_A, LOW);
    digitalWriteFast(LED_C, LOW);
}

/* http://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
returns change in encoder state (-1,0,1) */
int8_t readEncoder()
{
    static int8_t enc_states[] =
    {
        0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0
    }; // TODO use PROGMEM
    static uint8_t old_AB = 0;
    old_AB <<= 2; // remember previous state
    old_AB |= (ENC_PORT & 0x03); // add current state
    return (enc_states[(old_AB & 0x0f)]);
}

void setMode(uint8_t m)
{
//#if DEBUG
//    debugSerial << "SET_MODE:FROM:" << mode << ":TO:" << m;
//#endif
    mode = m;
}

void setState(uint8_t s)
{
#if DEBUG
    debugSerial << "SET_STATE:FROM:" << intState << ":TO:" << s;
#endif
    nextIntState = s;
}

void onStateClick(Button &b)
{
    if (tempoButton.holdTime() > 1500) // TODO constant
    {
        config.version = (uint16_t)VERSION;
        config.cycle = cycleTime;
        config.state = intState;

#if DEBUG
        debugSerial << "CONFIG:WRITE:" << config.state << " " << config.cpqn << " " << config.cycle << endl;
#endif

        // write to non-volatile memory
        EEPROM_writeAnything(0, config);

        ledLeft.notify(BICOLOR_GREEN, 1500, true);
        ledRight.notify(BICOLOR_GREEN, 1500, true);
        return;
    }

    if (b.holdTime() >= HOLD_THRESH)
    {
        switch (intState)
        {
        case STATUS_STOP:
            nextIntState = STATUS_START;
            ledRight.notify(BICOLOR_YELLOW, 150, true);
            break;
        }
    }
    else
    {
        switch (intState)
        {
        case STATUS_START:
        case STATUS_CONTINUE:
            // running, stop
            nextIntState = STATUS_STOP;
            break;
        case STATUS_STOP:
            nextIntState = STATUS_CONTINUE;
            break;
        }
    }
}

void onTempoClick(Button &b)
{
    /*
    // TODO midi panic
    if (stateButton.holdTime() > 1500)
    {
        ledLeft.notify(BICOLOR_RED, 1500, true);
        ledRight.notify(BICOLOR_RED, 1500, true);

        MIDI_PORT.write(0x79);
        delay(100);
        MIDI_PORT.write(0x7B);
        delay(1300);

        nextIntState = STATUS_STOP;
        // TODO MIDI PANIC
        return;
    }
    */
    if (b.holdTime() > HOLD_THRESH)
        // TODO show internal clock tepo on hold
        return;

    uint32_t tapCycle;
    static uint8_t tapCounter = 0;
    static uint32_t lastTapTime;
    static uint32_t tapTimer;
    static uint32_t tapTimeout;

    switch (mode)
    {
    case CLOCK_INTERNAL:
        // check for timer timeout
        if (micros() >= tapTimeout)
        {
            tapTimeFilter.reset();
            tapCounter = 1;
            //lastTapTime = micros();
            ledLeft.notify(BICOLOR_YELLOW, 75, true);
        }
        else
        {
            tapCounter++;
            tapTimer = micros() - lastTapTime;
            //lastTapTime = micros();
            tapCycle = tapTimeFilter.reading(tapTimer);
            if (tapCounter >= 4)
            {
                // set new cycle time
                cycleTime = tapCycle / config.cpqn;
                ledLeft.notify(BICOLOR_YELLOW, 75, true);
                ledRight.notify(BICOLOR_YELLOW, 75, true);
            }
            else
                ledLeft.notify(BICOLOR_YELLOW, 75, true);
        }
        lastTapTime = micros();
        tapTimeout = micros() + (config.cpqn * 100000);
        break;
    case CLOCK_EXTERNAL:
        setMode(CLOCK_SYNC);
        cpqnInt = cpqnExt;
        if (extState > 0)
            nextIntState = extState; // TODO CHECK
        ledRight.off();
        break;
    case CLOCK_SYNC:
        setMode(CLOCK_INTERNAL);
        ledLeft.off();
        break;
    }
}

extern void __attribute__((noreturn))
setup()
{
    // disable unused
    power_spi_disable();
    power_twi_disable();

    // begin of setup() - enable board led
    pinMode(BOARD_LED, OUTPUT);
    digitalWriteFast(BOARD_LED, HIGH);

    // configure input/output pins
    pinMode(ENC_A, INPUT);
    digitalWriteFast(ENC_A, HIGH);
    pinMode(ENC_B, INPUT);
    digitalWriteFast(ENC_B, HIGH);
    pinMode(GATE_PIN, OUTPUT);

#if DEBUG
    // setup software serial for debugging output
    pinMode(DEBUG_RX, INPUT);
    pinMode(DEBUG_TX, OUTPUT);
    debugSerial.begin(DEBUG_SPEED);
    // init terminal, clear screen, disable cursor
    debugSerial << "\x1b\x63" << "\x1b[2J" << "\x1b[?25l";
    // output some basic info
    debugSerial << endl << ID << ":" << VERSION << endl;
#endif

    // read configuration from non-volatile memory
    EEPROM_readAnything(0, config);

    // check if data has been written and loaded using the same firmware version
    // or if the reset combo is being pressed on startup
    if ((config.version != (uint16_t)VERSION) || (stateButton.isPressed() && tempoButton.isPressed()))
    {
        resetConfig();
    }

#if DEBUG
    debugSerial << "CONFIG:READ:" << config.state << " " << config.cpqn  << " " << config.cycle << endl;
    debugTime = millis() + DEBUG_INTERVAL;
#endif

    // copy values
    if (config.state == STATUS_CONTINUE)
        nextIntState = STATUS_START;
    else
        nextIntState = config.state;
    cycleTime = config.cycle;

    // initialize
    extTime = 0;
    intTime = 0;
    cpqnInt = 0;
    cpqnExt = 0;
    colorLeft = BICOLOR_RED;
    setMode(CLOCK_INTERNAL);

    // enable midi communication
    MIDI_PORT.begin(31250);

    // fancy led test effect
    uint8_t val = 0;
    do
    {
        val += 5;
        analogWrite(LED_B, val);
        analogWrite(LED_D, val);
        delay(4);
    }
    while (val < 192);
    do
    {
        val -= 5;
        analogWrite(LED_B, val);
        analogWrite(LED_D, val);
        delay(4);
    }
    while (val > 5);
    analogWrite(LED_B, 0);
    analogWrite(LED_D, 0);
    delay(150);

    tempoButton.clickHandler(onTempoClick);
    stateButton.clickHandler(onStateClick);

    // end of setup() - disable board led
    digitalWriteFast(BOARD_LED, LOW);
}

extern void __attribute__((noreturn))
loop()
{
    uint32_t diff = micros() - intTime;

    switch (mode)
    {
    case CLOCK_INTERNAL:
    case CLOCK_EXTERNAL:
        if (diff >= cycleTime)
        {
            MIDI_PORT.write(STATUS_SYNC);
            intTime = micros();
            if (++cpqnInt == config.cpqn)
            {
                digitalWriteFast(GATE_PIN, HIGH);
                ledRight.setColor(colorRight);
                cpqnInt = 0;
            }
            else if (cpqnInt == config.cpqn / 2)
            {
                digitalWriteFast(GATE_PIN, LOW);
                ledRight.off(true);
            }
        }
        break;
    }

    do
    {
        if ((MIDI_PORT.peek() & B10000000) != 0x80)
        {
            // flush
            MIDI_PORT.read();
        }
        else
        {
            const uint8_t status = MIDI_PORT.read();
            switch (status)
            {
            case STATUS_SYNC:
                diff = micros() - extTime;
                if (diff > 0)
                    cycleTimeExt = (uint32_t)cycleTimeExtFilter.reading(diff);
                extTime = micros();
                cpqnExt++;

                switch (mode)
                {
                case CLOCK_EXTERNAL:
                    if (cpqnExt == config.cpqn)
                    {
                        ledLeft.setColor(colorLeft, true);
                        cpqnExt = 0;
                    }
                    else if (cpqnExt == config.cpqn / 2)
                        ledLeft.off(true);
                    break;
                case CLOCK_SYNC:
                    MIDI_PORT.write(STATUS_SYNC);
                    if (cpqnExt == config.cpqn)
                    {
                        digitalWriteFast(GATE_PIN, HIGH);
                        ledLeft.setColor(colorLeft, true);
                        ledRight.setColor(colorRight, true);
                        cpqnExt = 0;
                    }
                    else if (cpqnExt == config.cpqn / 2)
                    {
                        digitalWriteFast(GATE_PIN, LOW);
                        ledLeft.off(true);
                        ledRight.off(true);
                    }
                    break;
                }
                break;
            case STATUS_STOP:
                if (mode == CLOCK_SYNC)
                {
                    MIDI_PORT.write(STATUS_STOP);
                    nextIntState = status;
                    colorRight = BICOLOR_RED;
                }
                extState = status;
                colorLeft = BICOLOR_RED;
                ledLeft.notify(BICOLOR_RED, 200, true);
                break;
            case STATUS_START:
                if (mode == CLOCK_SYNC)
                {
                    MIDI_PORT.write(STATUS_START);
                    nextIntState = status;
                    colorRight = BICOLOR_GREEN;
                }
                extState = status;
                colorLeft = BICOLOR_GREEN;
                ledLeft.notify(BICOLOR_YELLOW, 200, true);
                break;
            case STATUS_CONTINUE:
                if (mode == CLOCK_SYNC)
                {
                    MIDI_PORT.write(STATUS_CONTINUE);
                    colorRight = BICOLOR_GREEN;
                    nextIntState = status;
                }
                extState = status;
                colorLeft = BICOLOR_GREEN;
                ledLeft.notify(BICOLOR_GREEN, 200, true);
                break;
            }
        }
    }
    while (MIDI_PORT.available() > 0);

    if ((micros() - extTime >= CLOCK_DETECTION_WINDOW))
    {
        extTime = 0;
        cpqnExt = 0;
        setMode(CLOCK_INTERNAL);
        ledLeft.off();
    }
    else if (extTime > 0) // TODO logic expression
    {
        if (mode == CLOCK_INTERNAL)
        {
            setMode(CLOCK_EXTERNAL);
            colorLeft = BICOLOR_YELLOW;
        }
    }

    // button library
    tempoButton.process();
    stateButton.process();

    static int8_t counter;
    const int8_t encData = readEncoder();
    if (encData)
    {
        uint16_t inc;
        if (tempoButton.holdTime() > HOLD_THRESH)
            inc = 500;
        else
            inc = 10;
        // TODO add acceleration feature
        counter += encData;

        switch (mode)
        {
        case CLOCK_INTERNAL:
        case CLOCK_EXTERNAL:
            // TODO fix logic (UP/DOWN flag)
            switch (counter)
            {
            case 4:
                counter = 0;
                if (cycleTime < 70000)
                {
                    cycleTime += inc; // TODO acceleration
                    ledRight.notify(BICOLOR_YELLOW, 25, true);
                }
                break;
            case -4:
                counter = 0;
                if (cycleTime > 5000)
                {
                    cycleTime -= inc; // TODO acceleration
                    ledRight.notify(BICOLOR_YELLOW, 25, true);
                }
                break;
            }
            break;
        }
    }

    // compare
    if (intState != nextIntState)
    {
        switch (nextIntState)
        {
        case STATUS_STOP:
            MIDI_PORT.write(STATUS_STOP);
            colorRight = BICOLOR_RED;
            break;
        case STATUS_START:
            MIDI_PORT.write(STATUS_START);
            colorRight = BICOLOR_GREEN;
            break;
        case STATUS_CONTINUE:
            MIDI_PORT.write(STATUS_CONTINUE);
            colorRight = BICOLOR_GREEN;
            break;
        }
#if DEBUG
        debugSerial << "INT_STATE:FROM:" << intState << ":TO:" << nextIntState << endl;
#endif
        // store state (for comparison on next round)
        intState = nextIntState;
        ledRight.setColor(colorRight);
    }

    // led library
    ledLeft.drive();
    ledRight.drive();

#if DEBUG
    if ((millis() - debugTime) >= DEBUG_INTERVAL)
    {
        printDiag();
        debugTime = millis();
    }
    loopCycle = micros();
#endif
}

// end
