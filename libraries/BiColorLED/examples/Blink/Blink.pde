#include <BiColorLED.h>

// Simple demo of blinking using BiColorLED library
// Requires v1.1 or greater
// (C) 2012 Wolfgang Faust

// To see the effects of this example, just plug a bi-color LED
// into pins 4 and 5, along with the appropriate resistors.

BiColorLED led=BiColorLED(4,5); // (pin 1, pin 2)
unsigned long lb; // Time the colour was last changed

void setup() {
  Serial.begin(9600);
  led.setColor(1);
  led.setColor2(2);
  led.setBlinkSpeed(1000);
}

void loop() {
  // led.drive() MUST be called for blinking (and yellow) to work.
  led.drive();
  if (lb != led.lastBlink) {
    Serial.print(led.blinkSpeed);
    Serial.print(":");
    Serial.println(led.lastBlink);
    lb=led.lastBlink;
  }
}
