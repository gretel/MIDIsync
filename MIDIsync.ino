/*--------------------------------------------------------------------------------------------------
 "Hensel MODEL-01"
 Arduino "MIDIsync" - MIDI Master Clock Generator
 https://github.com/gretel/MIDIsync
 ɔ 2012-2014 Tom Hensel <tom@interpol8.net> Hamburg, Germany
 CC BY-SA 3.0 http://creativecommons.org/licenses/by-sa/3.0/
 --------------------------------------------------------------------------------------------------*/

// id
#define ID "MIDI_MASTER_CLOCK"
#define VERSION 27082013
#define DEBUG 0

// includes
#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <avr/power.h>
#include "Arduino.h"

// 3rd party includes
#include "digitalWriteFast.h"
#include "Streaming.h"
#include "movingAvg.h"
#include "BiColorLED.h"
#include "Button.h"

#define ENCODER_DO_NOT_USE_INTERRUPTS 1
#include "Encoder.h"

#if DEBUG
#include <SoftwareSerial.h>
#define DEBUG_RX 11
#define DEBUG_TX 12
#define DEBUG_SPEED 38400
#define DEBUG_INTERVAL 300
#endif

// hardware
#define BOARD_LED 13
#define MIDI_PORT Serial
#define MIDI_BPS 31250
#define BTN_ENCODER 2
#define BTN_STATE   4
#define GATE_PIN 8
#define ENC_A 14
#define ENC_B 15
#define ENC_PORT PINC
#define LED_A 9
#define LED_B 10
#define LED_C 5
#define LED_D 6

// states
#define CLOCK_INTERNAL 1
#define CLOCK_EXTERNAL 2
#define CLOCK_SYNC     3

// timings
#define CLOCK_DETECTION_WINDOW 600000 // TODO description
#define HOLD_THRESH 500 // TODO description

// midi status
// TODO rename
#define STATUS_SYNC     0xF8
#define STATUS_START    0xFA
#define STATUS_CONTINUE 0xFB
#define STATUS_STOP     0xFC
// TODO rename
#define MIDI_PROGRAM_CHANGE    0xC0
#define MIDI_CHANNEL_PRESSURE  0xD0

struct config_t
{
    uint16_t version;
    uint32_t cycle;
    uint8_t state;
    uint8_t cpqn;
    uint8_t thru;
} config;

// variables
uint8_t mode; // TODO struct
uint8_t intState; // TODO struct
uint8_t nextIntState; // TODO struct
uint8_t extState; // TODO struct
uint32_t cycleTime;
uint32_t cycleTimeExt;
uint8_t colorRight;
uint8_t colorLeft;
// utilize hardware registers
register uint32_t intTime asm("r2");
register uint32_t extTime asm("r6");
register uint8_t cpqnInt asm("r10");
register uint8_t cpqnExt asm("r11");
// variables depending on libraries
movingAvg tapTimeFilter;
movingAvg cycleTimeExtFilter;
Button stateButton = Button(BTN_STATE, BUTTON_PULLDOWN, true, 30);
Button tempoButton = Button(BTN_ENCODER, BUTTON_PULLDOWN, true, 30);
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
                << " THRU: " << config.thru
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
    // lights on
    digitalWriteFast(LED_A, HIGH);
    digitalWriteFast(LED_C, HIGH);
    // initialize configuration structure
    config.version = (uint16_t)VERSION;
    config.cpqn = 24;
    config.cycle = 20000;
    config.state = STATUS_STOP;
    // TODO DEBUG !!!
    config.thru = true;
    // write to non-volatile memory
    //EEPROM_writeAnything(0, config);
    eeprom_write_block((const void*)&config, (void*)0, sizeof(config));
    do
    {
        delay(100); // paranoia
    }
    while (stateButton.isPressed() || tempoButton.isPressed());
    // lights off
    digitalWriteFast(LED_A, LOW);
    digitalWriteFast(LED_C, LOW);
}

void writeConfig()
{
    config.version = (uint16_t)VERSION;
    config.cycle = cycleTime;
    config.state = intState;
#if DEBUG
    debugSerial << "CONFIG:WRITE:" << config.state << " " << config.cpqn << " " << config.cycle << endl;
#endif
    // write to non-volatile memory
    //EEPROM_writeAnything(0, config);
    eeprom_write_block((const void*)&config, (void*)0, sizeof(config));
    delay(100); // paranoia
    // give feedback
    ledLeft.notify(BICOLOR_GREEN, 1500, true);
    ledRight.notify(BICOLOR_GREEN, 1500, true);
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
#if DEBUG
    if (mode != m)
        debugSerial << "SET_MODE:FROM:" << mode << ":TO:" << m;
#endif
    mode = m;
}

void onStateClick(Button &b)
{
    if (tempoButton.holdTime() > HOLD_THRESH * 3)
    {
        writeConfig();
        return;
    }
    // TODO buggy?
    if (b.holdTime() > HOLD_THRESH * 3)
    {
        // toggle
        config.thru = !config.thru;

        switch (config.thru)
        {
            case true:
                ledLeft.notify(BICOLOR_GREEN, 333, true);
                break;
            case false:
                ledLeft.notify(BICOLOR_RED, 333, true);
                break;
        }
#if DEBUG
        debugSerial << "THRU:" << config.thru;
#endif
    }
    else if (b.holdTime() > HOLD_THRESH)
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
    if (b.holdTime() > HOLD_THRESH)
        // TODO show internal clock tempo on hold
        return;

    switch (mode)
    {
        case CLOCK_INTERNAL:
            static uint8_t tapCounter = 0;
            static uint32_t lastTapTime;
            static uint32_t tapTimer;
            static uint32_t tapTimeout;

            // check for timer timeout
            if (micros() > tapTimeout)
            {
                // restart tap counter
                tapTimeFilter.reset();
                tapCounter = 1;
                // give feedback
                ledLeft.notify(BICOLOR_YELLOW, 75, true);
            }
            else
            {
                // calculate
                tapCounter++;
                tapTimer = micros() - lastTapTime;
                //lastTapTime = micros();
                const uint32_t tapCycle = tapTimeFilter.reading(tapTimer);
                if (tapCounter >= 4)
                {
                    // set new cycle time
                    // TODO ensure minimum/maximum
                    cycleTime = tapCycle / config.cpqn;
                    // give feedback
                    ledLeft.notify(BICOLOR_YELLOW, 75, true);
                    ledRight.notify(BICOLOR_YELLOW, 75, true);
                }
                else
                    // give feedback
                    ledLeft.notify(BICOLOR_YELLOW, 75, true);
            }
            // store current time
            lastTapTime = micros();
            tapTimeout = micros() + (config.cpqn * 100000);
            break;
        case CLOCK_EXTERNAL:
            // change mode
            setMode(CLOCK_SYNC);
            // copy
            cpqnInt = cpqnExt;
            if (extState > 0)
                nextIntState = extState; // TODO CHECK
            ledRight.off();
            break;
        case CLOCK_SYNC:
            // change mode
            setMode(CLOCK_INTERNAL);
            ledLeft.off();
            break;
    }
}



uint8_t messageAvailable()
{

    const uint8_t avail = MIDI_PORT.available();
   switch (avail) {
    case 1 ... 8:                       // digits 0-8 (0 & 9 not used)
      // stuff
     break;
     } 

    if (avail > 0 && config.thru)
    {
#if DEBUG
        debugSerial << "MIDI:THRU:" << avail << endl;
#endif
        MIDI_PORT.write(MIDI_PORT.peek());
    }
    return avail;
}
/*
void sendMessage(uint8_t command, uint8_t channel, uint8_t param1, uint8_t param2)
{
    MIDI_PORT.write(command | (channel & 0x0F));
    MIDI_PORT.write(param1 & 0x7F);
    MIDI_PORT.write(param2 & 0x7F);
}
*/
/*
midi_msg_t readMessage()
{
    midi_msg_t message;
    uint8_t midi_status = MIDI_PORT.read();
    message.command = (midi_status & B11110000);
    message.channel = (midi_status & B00001111);
    message.param1  = MIDI_PORT.read();
    if (message.command != MIDI_PROGRAM_CHANGE && message.command != MIDI_CHANNEL_PRESSURE)
    {
        message.param2 = MIDI_PORT.read();
    }
    return message;
}
*/
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
    //EEPROM_readAnything(0, config);
    eeprom_read_block((void*)&config, (void*)0, sizeof(config));
    // check if data has been written and loaded using the same firmware version
    // or if the reset combo is being pressed on startup
    if ((config.version != (uint16_t)VERSION) || (stateButton.isPressed() && tempoButton.isPressed()))
    {
        resetConfig();
    }
#if DEBUG
    debugSerial << "CONFIG:READ:" << config.state << " " << config.cpqn << " " << config.cycle << " " << config.thru << endl;
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
    MIDI_PORT.begin(MIDI_BPS);
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
    // assign callbacks
    tempoButton.clickHandler(onTempoClick);
    stateButton.clickHandler(onStateClick);
    // end of setup() - disable board led
    digitalWriteFast(BOARD_LED, LOW);
}

extern void __attribute__((noreturn))
loop()
{
    switch (mode)
    {
        case CLOCK_INTERNAL:
        case CLOCK_EXTERNAL:
            // time to clock?
            if ((micros() - intTime) > cycleTime)
            {
                MIDI_PORT.write(STATUS_SYNC);
                //                MIDI_PORT.flush();
                // store current time
                intTime = micros();
                // clocks per quarter note
                if (++cpqnInt == config.cpqn)
                {
                    digitalWriteFast(GATE_PIN, HIGH);
                    cpqnInt = 0;
                    ledRight.setColor(colorRight);
                }
                else if (cpqnInt == config.cpqn / 2)
                {
                    digitalWriteFast(GATE_PIN, LOW);
                    ledRight.off(true);
                }
            }
            break;
    }

    uint32_t diff;
    while (messageAvailable() > 0)
    {
        const uint8_t command = MIDI_PORT.read() & B11110000;
#if DEBUG
        debugSerial << "MIDI:COMMAND:" << command << " " << endl;
#endif
        switch (command)
        {
            case STATUS_SYNC:
                diff = micros() - extTime; // TODO optimize
                if (diff > 0)
                    cycleTimeExt = (uint32_t)cycleTimeExtFilter.reading(diff);
                    //cycleTimeExt = diff;
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
                        //                        MIDI_PORT.flush();
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
                    nextIntState = command;
                    colorRight = BICOLOR_RED;
                }
                extState = command;
                colorLeft = BICOLOR_RED;
                ledLeft.notify(BICOLOR_RED, 200, true);
                break;
            case STATUS_START:
                if (mode == CLOCK_SYNC)
                {
                    nextIntState = command;
                    colorRight = BICOLOR_GREEN;
                }
                extState = command;
                colorLeft = BICOLOR_GREEN;
                ledLeft.notify(BICOLOR_YELLOW, 200, true);
                break;
            case STATUS_CONTINUE:
                if (mode == CLOCK_SYNC)
                {
                    colorRight = BICOLOR_GREEN;
                    nextIntState = command;
                }
                extState = command;
                colorLeft = BICOLOR_GREEN;
                ledLeft.notify(BICOLOR_GREEN, 200, true);
                break;
        }
    }
    if ((micros() - extTime > CLOCK_DETECTION_WINDOW))
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

    MIDI_PORT.flush(); // TODO test

    // button library
    tempoButton.process();
    stateButton.process();

    static int8_t counter;
    int8_t encData = readEncoder();
    if (encData)
    {
        uint16_t inc;
        // TODO add acceleration curve feature
        if (tempoButton.holdTime() > 100) // TODO constant
            inc = 500; // TODO constant
        else
            inc = 10; // TODO constant
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
                            ledLeft.notify(BICOLOR_RED, 25, true);
                        }
                        break;
                    case -4:
                        counter = 0;
                        if (cycleTime > 9000)
                        {
                            cycleTime -= inc; // TODO acceleration
                            ledLeft.notify(BICOLOR_GREEN, 25, true);
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
    if ((millis() - debugTime) > DEBUG_INTERVAL)
    {
        printDiag();
        debugTime = millis();
    }
    loopCycle = micros();
#endif
}

// end
