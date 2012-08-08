#include <digitalWriteFast.h>
#include <Button.h>
#include <LED.h>

/*
--------------------------------------------------------------------------------------------------
 MIDI Master Clock Generator
 Tom Hensel <tom@interpol8.net>
 inspired by Gijs Gieskes 2009 http://gieskes.nl
 --------------------------------------------------------------------------------------------------
 */
static const int STATUS_SYNC                  = 0xF8;
static const int STATUS_START                 = 0xFA;
static const int STATUS_CONTINUE              = 0xFB;
static const int STATUS_STOP                  = 0xFC;
static const int STATUS_RESET                 = 0xFF;

#define POT 0
#define BUTTON_TOGGLE  2
#define LED_BOARD  13
#define LED_CLOCK 6

boolean enabled = false;
boolean toggleFlag = false;
boolean ledFlag = LOW;
unsigned long time = 0;
unsigned long analogValueA = 0;
unsigned int ledCounter = 0;
byte cmd;
byte data1;
byte data2;
LED boardLED = LED(LED_BOARD);
LED clockLED = LED(LED_CLOCK);
Button button = Button(BUTTON_TOGGLE, BUTTON_PULLUP);

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

  pinMode(BUTTON_TOGGLE, INPUT);
  digitalWrite(BUTTON_TOGGLE, HIGH); 

  boardLED.blink(75,6);

  // midi rate
  Serial.begin(31250);

  if(button.isPressed())
  {
    Serial.write(STATUS_STOP);
    delay(250);
    Serial.write(STATUS_RESET);
    delay(250);
  }
}

void loop(){
  if(Serial.available() > 0)
  {
    // TODO simplify (no vars)
    cmd = Serial.read();
    data1 = Serial.read();
    data2 = Serial.read();

    Serial.write(cmd);
    Serial.write(data1);
    Serial.write(data2);
  }

  if(button.isPressed()){
    enabled = !enabled;

    if(enabled){
      Serial.write(STATUS_START);
    }
    else{
      Serial.write(STATUS_STOP);
      if(ledFlag)
      {
        clockLED.fadeOutTo(150, 64);
      }
    }
  }
  else
  {
    // map(value, fromLow, fromHigh, toLow, toHigh)
    analogValueA = map(analogRead(POT), 0, 1023, 128, 20000);  // delay settings

    if(micros() - time > analogValueA){
      time = micros();

      if(enabled){
        // midi clock
        Serial.write(STATUS_SYNC);

        ledCounter++;
        if(ledCounter > 3){
          ledCounter = 0;
          // toggle
          ledFlag = !ledFlag;
        }

        if(ledFlag){
          digitalWriteFast(LED_CLOCK, LOW);
        }
        else
        {
          digitalWriteFast(LED_CLOCK, HIGH);
        }
      }
    }
  }

}



