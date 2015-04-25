/*--------------------------------------------------------------------------------------------------
 "Hensel CLOCK-01"
 Arduino "MIDIsync" - MIDI Master Clock Generator
 https://github.com/gretel/MIDIsync
 ɔ 2012-2015 Tom Hensel <tom@jitter.eu> Hamburg, Germany
 CC BY-SA 3.0 http://creativecommons.org/licenses/by-sa/3.0/
 --------------------------------------------------------------------------------------------------*/

// id
#define ID "HENSEL_CLOCK-01"
#define VERSION 2504201501
#define DEBUG 0

// TODO: description
#define MIDI_PORT Serial
#define MIDI_BPS 31250
#define BTN_TEMPO 2
#define BTN_STATE 4
#define ENC_A 14
#define ENC_B 15
#define ENC_PORT PINC
#define GATE_PIN 8
#define BOARD_LED 13
#define LED_A 5
#define LED_B 6
#define LED_C 9
#define LED_D 10

// button debounce time
#define BTN_DBNC 25
// button hold threshold
#define BTN_HOLD_THRESH 220
// each encoder click corresponds to a value
#define ENCODER_VAL 16
// when pushed down (shif)
#define ENCODER_VAL_SHIFT 96
// min/max cycle duration
#define CYCLE_TIME_MIN 7200
#define CYCLE_TIME_MAX 115200
// number of data points
#define TAP_FILTER_POINTS 8
// time window to count beats as subsequent
#define TAP_WINDOW 2000000
// TODO: add comments
#define DEFAULT_CPQN 24
#define DEFAULT_CYCLE_TIME 20000
#define DEFAULT_MIDI_CH 9

#if DEBUG
#define DEBUG_RX 11
#define DEBUG_TX 12
#define DEBUG_SPEED 38400
#define DEBUG_INTERVAL 500000
#endif

// includes
#include <avr/eeprom.h>
#include <avr/power.h>
#include <avr/wdt.h>
#include <Arduino.h>

#if DEBUG
#include <SoftwareSerial.h>
#endif
// https://github.com/MajenkoLibraries/Average
#include "Average.h"
// https://github.com/JChristensen/Button
#include "Button.h"
// https://github.com/mpflaga/Arduino-digitalWriteFast
#include "digitalWriteFast.h"
// https://github.com/jonblack/arduino-fsm
#include "Fsm.h"
// https://github.com/jgillick/arduino-LEDFader
#include "LEDFader.h"
#include "Curve.h"
// https://github.com/oogre/StackArray
#include "StackArray.h"
// http://arduiniana.org/libraries/streaming/
#include "Streaming.h"
// https://github.com/FortySevenEffects/arduino_midi_library
#include "MIDI.h"

// MIDI library settings
struct MidiSettings : public midi::DefaultSettings
{
   static const bool UseRunningStatus = false;
   static const bool Use1ByteParsing = true;

};
// instantiate interface
MIDI_CREATE_CUSTOM_INSTANCE(HardwareSerial, Serial, midi_if, MidiSettings);

// use hardware registers for frequent changing values
register uint8_t counter asm("r2");
register uint8_t cpqn asm("r3");
register uint32_t cycle asm("r4");

// the state the machine is currently in
uint8_t state;
// the state the machine will change to during the next cycle
uint8_t nextState;
// definiton of states
enum states_t { STATE_HALT, STATE_STOP, STATE_START, STATE_CONTINUE };
State state_halt(STATE_HALT, &s_halt_enter, NULL);
State state_stop(STATE_STOP, &s_stop_enter, NULL);
State state_start(STATE_START, &s_start_enter, NULL);
State state_continue(STATE_CONTINUE, &s_continue_enter, NULL);
Fsm machine(&state_stop);

// this data is going to saved to and loaded from the EEPROM
struct config_t
{
    uint32_t version;
    uint32_t cycle;
    uint8_t autostart;
    uint8_t cpqn;
} config;

// holding a key will give the press of another a different meaning
uint8_t inputContext;
enum input_t { BUTTON_CLICK, BUTTON_TRANSPORT_HOLD };

// one button to tap tempo (encoder press)
Button tempoButton = Button(BTN_TEMPO, false, false, BTN_DBNC);
// and one to control the transport
Button transportButton = Button(BTN_STATE, false, false, BTN_DBNC);
// smooth changing the tempo on taps
Average <uint32_t> clockFilter(TAP_FILTER_POINTS);

// total amount of LEDs
// two LEDs with two emitters each (bicolor)
#define LED_NUM 4
// give a name to each
enum leds_t { LED_LEFT_A, LED_LEFT_B, LED_RIGHT_A, LED_RIGHT_B };
// definiton of LED interfaces
LEDFader leds[LED_NUM] = {
  LEDFader(LED_A),
  LEDFader(LED_B),
  LEDFader(LED_C),
  LEDFader(LED_D)
};

// an event of light (control LEDs)
struct led_event
{
    uint8_t id;
    uint8_t val;
    uint16_t dur;
};
// priority queueing for light emission events (whoa)
enum stack_action_t { ACT_PUSH, ACT_UNSHIFT, ACT_REPLACE };
// events will be stacked and processed when applicable
StackArray <led_event> stack;

#if DEBUG
// initialize secondary serial port for debbuging output
// WARNING: SoftwareSerial adds lots of latency - timming will jitter!
SoftwareSerial debugSerial(DEBUG_RX, DEBUG_TX);
uint32_t debugTime;
uint32_t loopCycle;

void printDiag()
{
    const uint16_t duration = micros() - loopCycle;
    debugSerial
    << " COUNT: " << micros()
    << " LOOP: " << "\033[4m" << duration << "\033[0m"
    << " STATE: " << "\033[7m" << state << "\033[0m"
    << " COUNTER: " << counter
    << " CYCL_DUR: " << cycle
    << " INPUT_STATE: " << inputContext
    << endl;
}
#endif

void reset()
{
    // use watchdog timer for reset
    wdt_disable();
    wdt_enable(WDTO_15MS);
    while (1) {}
}

void alert()
{
    // light up all LEDs
    for (uint8_t i = 0; i < LED_NUM; i++) {
        LEDFader *led = &leds[i];
        led->set_value(255);
        led->update();
    }
}

void awaitRelease()
{
    do
    {
        // wait for buttons to be released so we can finally get on
        delay(100);
        transportButton.read();
        tempoButton.read();
    }
    while (transportButton.isPressed() || tempoButton.isPressed());
}

void writeEprom()
{
    #if DEBUG
    debugSerial << "WRITE_EPROM" << endl;
    #endif
    digitalWriteFast(BOARD_LED, HIGH);
    // actually write
    eeprom_write_block((const void*)&config, (void*)0, sizeof(config));
    delay(100); // paranoia
    digitalWriteFast(BOARD_LED, LOW);
}

void resetConfig()
{
    #if DEBUG
    debugSerial << "RESET_CONFIG" << endl;
    #endif
    // initialize configuration structure
    config.version = (uint32_t)VERSION;
    config.cpqn = (uint8_t)DEFAULT_CPQN;
    config.cycle = (uint32_t)DEFAULT_CYCLE_TIME;
    config.autostart = STATE_STOP;
    writeEprom();
}

void saveConfig()
{
    config.version = (uint32_t)VERSION;
    config.cpqn = cpqn;
    config.cycle = cycle;
    // enforce valid state so we won't hang on startup
    switch (state)
    {
        case STATE_START:
        case STATE_CONTINUE:
            config.autostart = STATE_START;
            break;
        default:
            config.autostart = STATE_STOP;
            break;
    }
    #if DEBUG
    debugSerial << "SAVE_CONFIG:VERSION => " << config.version << " AUTOSTART => " << config.autostart << " CPQN =>" << config.cpqn << " CYCLE => " << config.cycle << endl;
    #endif
    writeEprom();
}

void loadConfig()
{
    // read configuration
    eeprom_read_block((void*)&config, (void*)0, sizeof(config));
    #if DEBUG
    debugSerial << "LOAD_CONFIG:VERSION => " << config.version << " AUTOSTART => " << config.autostart << " CPQN =>" << config.cpqn << " CYCLE => " << config.cycle << endl;
    #endif
}

/* http://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
returns change in encoder state (-1,0,1) */
int8_t readEncoder()
{
    static int8_t enc_states[] =
    {
        0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0
    };
    static uint8_t old_AB;
    old_AB <<= 2; // remember previous state
    old_AB |= (ENC_PORT & 0x03); // add current state
    return (enc_states[(old_AB & 0x0f)]);
}

void setState(uint8_t s)
{
    #if DEBUG
    debugSerial << "SET_STATE:" << s << endl;
    #endif
    nextState = s;
}

void setCycle(uint32_t t)
{
    t = constrain(t, CYCLE_TIME_MIN, CYCLE_TIME_MAX);
    #if DEBUG
    debugSerial << "SET_CYCLE:" << cycle << "=>" << t << endl;
    #endif
    cycle = t;
}

void queueLight(uint8_t a, uint8_t id, uint8_t val, uint16_t dur)
{
    // compose event
    led_event e;
    e.id = id;
    e.val = val;
    e.dur = dur;
    switch(a)
    {
        case ACT_PUSH:
            // add to stack (ordered queue)
            stack.push(e);
            break;
        case ACT_UNSHIFT:
            // add to top of stack (priority)
            stack.unshift(e);
            break;
        case ACT_REPLACE:
            // clear stack and add (exclusive)
            while (!stack.isEmpty()) stack.pop();
            stack.push(e);
            break;
    }
}

void s_halt_enter()
{
    midi_if.sendRealTime(midi::Stop);
    queueLight(ACT_REPLACE, LED_LEFT_A, 0, 100);
    queueLight(ACT_PUSH, LED_LEFT_B, 0, 100);
    queueLight(ACT_PUSH, LED_RIGHT_A, 220, 150);
    queueLight(ACT_PUSH, LED_RIGHT_B, 0, 100);
}

void s_stop_enter()
{
    midi_if.sendRealTime(midi::Stop);
    queueLight(ACT_REPLACE, LED_RIGHT_A, 255, 150);
    queueLight(ACT_PUSH, LED_RIGHT_B, 0, 100);
}

void s_start_enter()
{
    midi_if.sendRealTime(midi::Start);
    queueLight(ACT_REPLACE, LED_RIGHT_A, 0, 100);
    queueLight(ACT_PUSH, LED_RIGHT_B, 255, 150);
}

void s_continue_enter()
{
    midi_if.sendRealTime(midi::Continue);
    queueLight(ACT_REPLACE, LED_RIGHT_A, 200, 100);
    queueLight(ACT_PUSH, LED_RIGHT_B, 255, 250);
}

extern void __attribute__((noreturn))
setup()
{
    // be paranoid in regards to latency
    power_spi_disable();
    power_twi_disable();
    power_adc_disable();

    // begin of setup() - enable board LED
    pinMode(BOARD_LED, OUTPUT);
    digitalWriteFast(BOARD_LED, HIGH);

    // configure input/output pins
    pinMode(ENC_A, INPUT);
    digitalWriteFast(ENC_A, HIGH);
    pinMode(ENC_B, INPUT);
    digitalWriteFast(ENC_B, HIGH);
    pinMode(GATE_PIN, OUTPUT);

    #if DEBUG
    // setup secondary serial
    pinMode(DEBUG_RX, INPUT);
    pinMode(DEBUG_TX, OUTPUT);
    debugSerial.begin(DEBUG_SPEED);
    // identifcation please
    debugSerial << ID << ":" << VERSION << endl;
    #endif

    // setup each LED
    for (uint8_t i = 0; i < LED_NUM; i++) {
        LEDFader *led = &leds[i];
        // smooth fading
        led->set_curve(&Curve::exponential);
    };

    // setup state transitions
    machine.add_transition(&state_start, &state_stop,    STATE_STOP, NULL);
    machine.add_transition(&state_continue, &state_stop, STATE_STOP, NULL);
    machine.add_transition(&state_halt, &state_stop,     STATE_STOP, NULL);
    machine.add_transition(&state_start, &state_halt,    STATE_HALT, NULL);
    machine.add_transition(&state_continue, &state_halt, STATE_HALT, NULL);
    machine.add_transition(&state_stop, &state_start,    STATE_START, NULL);
    machine.add_transition(&state_stop, &state_continue, STATE_CONTINUE, NULL);

    // load configuration from non-volatile memory (5th dimensional eternity like in interstellargh)
    loadConfig();
    // check if data has been written and loaded using the same firmware version
    // or if the reset combo is being pressed on startup
    if ((config.version != (uint32_t)VERSION) || (transportButton.isPressed() && tempoButton.isPressed()))
    {
        alert();
        // actually reset
        resetConfig();
        awaitRelease();
        reset();
    }

    // MIDI you, MIDI me!
    midi_if.begin();
    midi_if.turnThruOff();
    midi_if.setInputChannel(DEFAULT_MIDI_CH);
    // 'If a Tune Request command is sent, all the MIDI instruments in the system that have a tuning routine
    // will give themselves a quick checkover and retune to their own internal reference'
    // http://www.soundonsound.com/sos/1995_articles/oct95/midibasics3.html
    // do it and dream on
    // TODO check for side-effects
    midi_if.sendTuneRequest();

    // copy and set
    cpqn = config.cpqn;
    setCycle(config.cycle);
    // end of setup() - disable board led (oldschool diag)
    digitalWriteFast(BOARD_LED, LOW);
    // call enter handler of default state so we won't end up in nirvana
    machine.enter_handler();
    // recall saved state
    setState(config.autostart);
}

extern void __attribute__((noreturn))
loop()
{
    // will things change?
    if (state != nextState)
    {
        #if DEBUG
        debugSerial << "TRANSITION:" << state << "=>" << nextState << endl;
        #endif
        // yep, apply change
        state = nextState;
        machine.trigger(state);
    }

    // will store encoder's pulses
    static int8_t encCounter;
    // will store last time the clock cycled
    static uint32_t lastCycleAt;
    // time of last tap
    static uint32_t lastTapTime;
    // check if clock has passed
    const uint32_t intDiff = micros() - lastCycleAt;

    switch(state)
    {
        case STATE_HALT:
            // nop, don't clock
            break;
        case STATE_START:
        case STATE_CONTINUE:
        case STATE_STOP:
            if (intDiff >= cycle)
            {
                // yep, store
                lastCycleAt = micros();
                // output clock message
                midi_if.sendRealTime(midi::Clock);
                // count clicks per quarter note (cpqn)
                if(++counter == 1)
                {
                    // raise control voltage
                    digitalWriteFast(GATE_PIN, LOW);
                    // light tempo LEDs up
                    queueLight(ACT_PUSH, LED_LEFT_A, 240, 0);
                    queueLight(ACT_PUSH, LED_LEFT_B, 240, 0);
                }
                // 1/4 note counted?
                else if(counter == cpqn)
                {
                    // reset
                    counter = 0;
                }
                // 1/8 note counted (half-time)?
                else if(counter == cpqn / 4)
                {
                    // drop control voltage
                    digitalWriteFast(GATE_PIN, HIGH);
                    // turn tempo LEDs off
                    queueLight(ACT_PUSH, LED_LEFT_A, 0, 50);
                    queueLight(ACT_PUSH, LED_LEFT_B, 0, 50);
                }
            }

            uint16_t changeValue = ENCODER_VAL;
            // check if encoder is moved
            int8_t encData = readEncoder();
            if (encData)
            {
                // change value rapidly while button is pressed
                if (tempoButton.pressedFor(BTN_HOLD_THRESH))
                {
                    lastTapTime = 0;
                    changeValue = ENCODER_VAL_SHIFT;
                }

                // add to counter
                encCounter += encData;
                switch (encCounter)
                {
                    // turned left, increase (slower)
                    case 4:
                        encCounter = 0;
                        setCycle(cycle + changeValue);
                        queueLight(ACT_REPLACE, LED_LEFT_A, 200, 0);
                        break;
                    // turned right, decrease (faster)
                    case -4:
                        encCounter = 0;
                        setCycle(cycle - changeValue);
                        queueLight(ACT_REPLACE, LED_LEFT_B, 220, 0);
                        break;
                }
            }

            if(tempoButton.wasPressed())
            {
                if(lastTapTime > 0)
                {
                    // time passed since last beat
                    const uint32_t tapDiff = micros() - lastTapTime;
                    // check if cycle duration time gets larger
                    if(tapDiff < TAP_WINDOW)
                    {
                        // store value (and have it smoothied)
                        clockFilter.push(tapDiff);
                        // TODO optimize: division
                        const uint32_t newCycle = clockFilter.mean() / cpqn;
                        // yep, change cycle duration
                        if(newCycle - 500 > cycle)
                        {
                            queueLight(ACT_REPLACE, LED_LEFT_A, 220, 0);
                            queueLight(ACT_UNSHIFT, LED_LEFT_B, 0, 0);
                        }
                        else if(newCycle + 500 < cycle)
                        {
                            queueLight(ACT_REPLACE, LED_LEFT_A, 0, 0);
                            queueLight(ACT_UNSHIFT, LED_LEFT_B, 240, 0);
                        }
                        else
                        {
                            queueLight(ACT_REPLACE, LED_LEFT_A, 240, 0);
                            queueLight(ACT_UNSHIFT, LED_LEFT_B, 240, 0);
                        }
                        setCycle(newCycle);
                    }
                }
                // store current time
                lastTapTime = micros();
            }
            break;
    }

    // switch according to context
    switch(inputContext)
    {
        case BUTTON_CLICK:
            // check if transport button is being hold
            if (transportButton.pressedFor(BTN_HOLD_THRESH))
            {
                // yep, change context (consider next time)
                inputContext = BUTTON_TRANSPORT_HOLD;
            }
            else
            {
                // check if button has been pressed
                if (transportButton.wasReleased())
                {
                    // switch according to state
                    switch (state)
                    {
                        case STATE_START:
                        case STATE_CONTINUE:
                        case STATE_HALT:
                            setState(STATE_STOP);
                            break;
                        case STATE_STOP:
                            // TODO: check for side-effects
                            midi_if.sendSongPosition(0);
                            setState(STATE_START);
                            break;
                    }
                    // reset context
                    inputContext = BUTTON_CLICK;
                }
            }
            break;
        case BUTTON_TRANSPORT_HOLD:
            // check if button was released now
            if (transportButton.wasReleased())
            {
                switch(state)
                {
                    case STATE_STOP:
                        setState(STATE_CONTINUE);
                        break;
                    case STATE_START:
                    case STATE_CONTINUE:
                        setState(STATE_HALT);
                        break;
                }
                // reset context
                inputContext = BUTTON_CLICK;
            }
            // check for 'save config' combo
            else if (tempoButton.pressedFor(3000))
            {
                alert();
                saveConfig();
                setState(STATE_HALT);
                awaitRelease();
                reset();
            }
            break;
    }

    // check if somebody is knocking on the midi input
    if (midi_if.read())
    {
        // yep, watcha want
        const uint8_t command = midi_if.getType();
        // serve on some commands
        switch(command)
        {
            case midi::Stop:
                setState(STATE_STOP);
                break;
            case midi::Start:
                setState(STATE_START);
                break;
            case midi::Continue:
                setState(STATE_CONTINUE);
                break;
            // TODO: implement sysex
        }
    }

    // check if something is stacked
    if(!stack.isEmpty())
    {
        // yep, let's go for the event
        led_event l = stack.pop();
        // cast according LED interface
        LEDFader *led = &leds[l.id];
        // fade to color for duration of
        led->fade(l.val, l.dur);
    }

    // update drive of each LED
    for (uint8_t i = 0; i < LED_NUM; i++) {
        LEDFader *led = &leds[i];
        led->update();
    }

    // read buttons
    tempoButton.read();
    transportButton.read();

    #if DEBUG
    // output debugging stuff every few cycles
    if ((micros() - debugTime) >= DEBUG_INTERVAL)
    {
        printDiag();
        // store last time
        debugTime = micros();
    }
    loopCycle = micros();
    #endif
}

// end
