#include <Button.h>
#include <SoftwareSerial.h>
// http://code.google.com/p/digitalwritefast/
#include <digitalWriteFast.h>

/*
--------------------------------------------------------------------------------------------------
 MIDI Master Clock Generator
 CC BY-SA 3.0 - 2012 - Tom Hensel <tom@interpol8.net>
 inspired by Gijs Gieskes 2009 http://gieskes.nl
 --------------------------------------------------------------------------------------------------
 */

#define POT 0
#define BUTTON_TOGGLE 2
#define LED_BOARD 13
#define LED_CLOCK 6
#define SERIAL_RX 7
#define SERIAL_TX 8

// static
static const unsigned int STATUS_SYNC     = 0xF8;
static const unsigned int STATUS_START    = 0xFA;
static const unsigned int STATUS_CONTINUE = 0xFB;
static const unsigned int STATUS_STOP     = 0xFC;
static const unsigned int STATUS_RESET    = 0xFF;

static const unsigned int BUTTON_PRESS    = 1;
static const unsigned int BUTTON_HOLD     = 2;

// var
unsigned long time = 0;
unsigned long analogValueA = 0;
unsigned int currentState;
unsigned int nextState;
unsigned int buttonState;
byte cmd;
byte data1;
byte data2;
Button toggleButton = Button(BUTTON_TOGGLE, HIGH);
SoftwareSerial syncSerial(SERIAL_RX, SERIAL_TX); // RX, TX

//http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1208715493/11
#define FASTADC 1

// defines for setting and clearing register bits
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

void setup(){
#if FASTADC
  // set prescale to 16
  sbi(ADCSRA,ADPS2);
  cbi(ADCSRA,ADPS1);
  cbi(ADCSRA,ADPS0);
#endif

  // pin configuration
  pinMode(LED_BOARD, OUTPUT);
  pinMode(LED_CLOCK, OUTPUT);
  pinMode(SERIAL_RX, INPUT);
  pinMode(SERIAL_TX, OUTPUT);
  pinMode(BUTTON_TOGGLE, INPUT);

  syncSerial.begin(19200);
  syncSerial.println("SETUP");

  // button library
  toggleButton.setDebounceDelay(50);
  toggleButton.setHoldDelay(500);

  // decoration
  digitalWrite(LED_CLOCK, HIGH);
  digitalWrite(LED_BOARD, LOW);
  delay(100);
  digitalWrite(LED_CLOCK, LOW);
  digitalWrite(LED_BOARD, HIGH);
  delay(33);
  digitalWrite(LED_CLOCK, HIGH);
  digitalWrite(LED_BOARD, LOW);
  delay(100);
  digitalWrite(LED_CLOCK, LOW);
  digitalWrite(LED_BOARD, HIGH);
  delay(33);

  // midi rate
  Serial.begin(31250);

  // initialize
  currentState = nextState = STATUS_STOP;

  // debug
  syncSerial.println("READY");
}

void loop()
{
  // check
  if(Serial.available() > 0)
  {
    // read a byte
    cmd = Serial.read();

    // debug
    syncSerial.print("IN:");
    syncSerial.println(cmd);

    // check for commands we will obey
    if (cmd == STATUS_STOP || cmd == STATUS_START || cmd == STATUS_CONTINUE)
    {
      // change state according to command
      setNextState(cmd);
    }
    else if(cmd == STATUS_SYNC)
    {
      // disable clock if we receive sync data (either our own, or from a daw or so)
      // TODO TEST
      setNextState(STATUS_STOP);
    } 
    else {
      data1 = Serial.read();
      data2 = Serial.read();

      Serial.write(cmd);
      Serial.write(data1);
      Serial.write(data2);

      // debug
      syncSerial.print(":");
      syncSerial.print(data1);
      syncSerial.print(":");
      syncSerial.print(data2);
    }

    // debug
    syncSerial.println();
  }

  // button library
  toggleButton.listen();

  // check for button press (down)
  if (toggleButton.onPress()) {
    buttonState = BUTTON_PRESS;
  }

  // check if the button is being hold
  if (toggleButton.isHold()) {
    buttonState = BUTTON_HOLD;
  }

  // check for button release (up)
  if (toggleButton.onRelease()) {
    // ignore unbound states
    if(!buttonState) {
      return;
    }

    // debug
    syncSerial.print("RELEASE:");
    syncSerial.println(buttonState);

    // check if clock is disabled
    if(currentState == STATUS_STOP)
    {
      // single press, send continue command
      if(buttonState == BUTTON_PRESS)
      {
        setNextState(STATUS_CONTINUE);
      }
      // button has been hold down, send (re)start command
      else if(buttonState == BUTTON_HOLD)
      {
        setNextState(STATUS_START);
      }
    }
    // check if clock is enabled
    else if(currentState == STATUS_START || currentState == STATUS_CONTINUE)
    {
      // yep, stop
      setNextState(STATUS_STOP);
    }
    else // failsafe
    {
      setNextState(STATUS_STOP);
    }   
  }

  // compare
  if(currentState != nextState)
  {
    // store state (for comparison on next round)
    currentState = nextState;

    // debug
    syncSerial.print("STATE_CHANGE:");
    syncSerial.print(currentState);
    syncSerial.print("->");
    syncSerial.println(nextState);

    // check if clock is going to start
    if(nextState == STATUS_START || nextState == STATUS_CONTINUE)
    {
      // send start or continue command
      Serial.write(nextState);
      // ensure brightness
      digitalWriteFast(LED_CLOCK, HIGH);
    }
    else if(nextState == STATUS_STOP) 
    {
      // send stop command
      Serial.write(STATUS_STOP);
      // ensure darkness
      digitalWriteFast(LED_CLOCK, LOW);
    }
    else // failsafe
    {  
      currentState = STATUS_STOP;
    }
  }

  // check if clock is enabled
  if(currentState == STATUS_START || currentState == STATUS_CONTINUE)
  {
    // map(value, fromLow, fromHigh, toLow, toHigh)
    analogValueA = map(analogRead(POT), 0, 1023, 10000, 200000);  // delay settings

    if(micros() - time > analogValueA)
    {
      // midi clock
      Serial.write(STATUS_SYNC);

      // copy
      time = micros();
      digitalWriteFast(LED_CLOCK, HIGH);

      // debug
      syncSerial.println("SYNC");
    }
    else
    {
      digitalWriteFast(LED_CLOCK, LOW);
    }
  }
}

void setNextState(unsigned int state)
{
  // copy
  nextState = state;
  // DEBUG
  //syncSerial.print("SET_NEXT_STATE:");
  //debugPrintstate);
}

// end