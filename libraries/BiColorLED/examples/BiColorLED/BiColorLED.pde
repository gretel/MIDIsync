#include <BiColorLED.h>

// Simple demo of features available in V1.0 of the BiColorLED library
// (C) 2012 Wolfgang Faust

// To see the effects of this example, just plug a bi-color LED
// into pins 4 and 5, along with the appropriate resistors.

BiColorLED led=BiColorLED(4,5); // (pin 1, pin 2)
unsigned long time; // Time the colour was last changed

void setup() {
  // BiColorLED doesn't need anything here
}

void loop() {
  // Change the colour once each second; Order: 1, 3, 2
  if (millis()-time > 1000) {
    int color=led.getColor();
    if (color == 1) {
      color=3;
    } else if (color == 3) {
      color=2;
    } else if (color == 2) {
      color=1;
    } else { // Starts at 0, or in case anything bizzarre happens to the color var
      color=1;
    }
    led.setColor(color);
    time=millis(); // Keep track of time of colour change.
  }
  // led.drive() MUST be called frequently (at least once every 10 ms)
  // for the yellow to work. Otherwise, it will just stay red or green.
  // This means that you should avoid calling delay().
  // Before 10ms, there is no flicker visible
  // At ~10ms, there is a barely noticible `jitter'
  // At ~15ms, the flickering is quite obvious.
  led.drive();
}
