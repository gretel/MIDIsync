/*
--------------------------------------------------------------------------------------------------
 MIDI Master Clock Generator
 Tom Hensel <tom@interpol8.net>
 inspired by Gijs Gieskes 2009 http://gieskes.nl
 --------------------------------------------------------------------------------------------------
 */

// http://code.google.com/p/digitalwritefast/
#include <digitalWriteFast.h>

// pins
#define POT 0
#define BUTTON_TOGGLE  2
#define LED_BOARD  13
#define LED_CLOCK 6

// static
static const int STATUS_SYNC     = 0xF8;
static const int STATUS_START    = 0xFA;
static const int STATUS_CONTINUE = 0xFB;
static const int STATUS_STOP     = 0xFC;
static const int STATUS_RESET    = 0xFF;

// var
unsigned long time = 0;
unsigned long analogValueA = 0;
unsigned int ledCounter = 0;
boolean enabled = false;
boolean prevEnabled = false;
boolean ledFlag = LOW;
boolean prevLedFlag = LOW;
byte cmd;
byte data1;
byte data2;
int buttonState;   // the previous reading from the input pin
int lastButtonState = LOW;   // the previous reading from the input pin
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers

//LED boardLED = LED(LED_BOARD);
//LED clockLED = LED(LED_CLOCK);
//Button button = Button(BUTTON_TOGGLE, HIGH);

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

  pinMode(LED_BOARD, OUTPUT);
  pinMode(LED_CLOCK, OUTPUT);
  pinMode(BUTTON_TOGGLE, INPUT);

  // decoration
  digitalWriteFast(LED_CLOCK, HIGH);
  delay(250);
  digitalWriteFast(LED_CLOCK, LOW);

  // midi rate
  Serial.begin(31250);
}

void loop(){
  if(Serial.available() > 0)
  {
    // TODO non-blocking buffer
    cmd = Serial.read();
    data1 = Serial.read();
    data2 = Serial.read();

    Serial.write(cmd);
    Serial.write(data1);
    Serial.write(data2);
  }

  int reading = digitalRead(BUTTON_TOGGLE);

  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  } 
  if ((millis() - lastDebounceTime) > debounceDelay) {
    buttonState = reading;
  }
  // copy
  lastButtonState = reading;

  if(buttonState)
  {
    enabled = !enabled;
  }

  // compare to previous state
  if(prevEnabled != enabled){
    // copy
    prevEnabled = enabled;

    if(enabled)
    {
      // send start command
      Serial.write(STATUS_START);
      // ensure darkness
      digitalWriteFast(LED_CLOCK, HIGH);
    } 
    else{
      // send stop command
      Serial.write(STATUS_STOP);
      // ensure darkness
      digitalWriteFast(LED_CLOCK, LOW);
      ledFlag = LOW;
      delay(50);
    }
  }

  if(enabled)
  {
    // map(value, fromLow, fromHigh, toLow, toHigh)
    analogValueA = map(analogRead(POT), 0, 1023, 10, 100000);  // delay settings

    if(micros() - time > analogValueA){
      // copy
      time = micros();

      // midi clock
      Serial.write(STATUS_SYNC);

      // increment
      ledCounter++;
      // compare
      if(ledCounter > 3){
        // reset
        ledCounter = 0;
        // toggle
        ledFlag = !ledFlag;
      }

      // compare to previous state
      if(prevLedFlag != ledFlag){
        // copy
        prevLedFlag = ledFlag;
        // set output
        digitalWriteFast(LED_CLOCK, ledFlag);
      }
    }
  }
}

// end