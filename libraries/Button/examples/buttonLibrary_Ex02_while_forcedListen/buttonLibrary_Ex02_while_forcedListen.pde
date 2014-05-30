/*
 * Button Library Example 2 -  status check using an if statement.
 * ------------
 *
 * Circuit: 
 *          - Button, pulled up externally with 10k resitor on pin 2
 *          - Extra LED on pin 9
 *
 * Behavior: 
 *          LEDs depend on a button's press state using if statement
 *          When button is pressed the LED on 9 is on.
 *          When the button is released the LED on pin 13 is on
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
  
  //input mode is set in library
}

void loop()
{

  helloButton.listen();  

  //passing a 1 to the function forces it to
  //recheck the state of the button - implemented
  //for all events
  while (helloButton.isPressed(1)) {
    digitalWrite(ledPin1, HIGH);
    digitalWrite(ledPin2, LOW);
  }


  while (helloButton.isReleased(1)) {
    digitalWrite(ledPin2, HIGH);
    digitalWrite(ledPin1, LOW);
  }

}
