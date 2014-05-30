 /*
 * Button Library Example 7 -  toggling behavior
 * ------------
 *
 * Circuit: 
 *          - Button, pulled up externally with 10k resitor on pin 2
 *          - Extra LED on pin 9
 *
 * Behavior: 
 *          Gets toggle behavior from a momentary button. LEDs alternate
 *          between on and off every time the button actuated...
 *          LED on ledPin1 chacges with the press of the button
 *          LED on ledPin2 changes with the release
 *
 *
 * Created 13 April 2007
 * Updated 29 October 2010
 * by Carlyn Maw
 *
 */

#include <Button.h>

int ledPin1 = 9;
int ledPin2 = 13;

//Instantiate Button on digital pin 2
//pressed = ground (i.e. pulled high with external resistor)
Button helloButton = Button(2, LOW);

void setup()
{

  pinMode(ledPin1, OUTPUT);      // sets the digital pin as output
  pinMode(ledPin2, OUTPUT);      // sets the digital pin as output
  
  digitalWrite(ledPin1, HIGH);      // sets the digital pin on to prove a point
  digitalWrite(ledPin2, HIGH);      // sets the digital pin on to prove a point
}

void loop()
{
  helloButton.listen();

  //notice it STARTS OFF
  if (helloButton.onPressAsToggle()) {
    digitalWrite(ledPin1, HIGH);
  }  
  else { 
    digitalWrite(ledPin1, LOW); 
  }

  //notice it STARTS OFF
  if (helloButton.onReleaseAsToggle()) {
    digitalWrite(ledPin2, HIGH);
  }  
  else { 
    digitalWrite(ledPin2, LOW); 
  }


}

