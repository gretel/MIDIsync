/*
--------------------------------------------------------------------------------------------------
 MIDI Master Clock Generator
 Tom Hensel <tom@interpol8.net>
 inspired by Gijs Gieskes 2009 http://gieskes.nl
 --------------------------------------------------------------------------------------------------
 */

// Carlyn Maw
#include <Button.h>
// http://code.google.com/p/digitalwritefast/
#include <digitalWriteFast.h>
// Alexander Brevig
#include <LED.h>

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
LED boardLED = LED(LED_BOARD);
LED clockLED = LED(LED_CLOCK);
Button button = Button(BUTTON_TOGGLE, HIGH);

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

  //button.setDebounceDelay(50);
  //boardLED.blink(50,3);

  // decoration
  clockLED.fadeIn(150);

  // midi rate
  Serial.begin(31250);

  if(button.isPressed())
  {
    // send stop first..
    Serial.write(STATUS_STOP);
    delay(250);
    // then reset
    Serial.write(STATUS_RESET);
    delay(250);
  }

  // decoration
  clockLED.fadeOut(150);
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

  button.listen();

  // detect button press
  if(button.onPressAsToggle()){
    // toggle
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
    } 
    else{
      // send stop command
      Serial.write(STATUS_STOP);
      // ensure darkness
      digitalWriteFast(LED_CLOCK, LOW);
    }
  }

  if(enabled)
  {
    // map(value, fromLow, fromHigh, toLow, toHigh)
    analogValueA = map(analogRead(POT), 0, 1023, 20, 100000);  // delay settings

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