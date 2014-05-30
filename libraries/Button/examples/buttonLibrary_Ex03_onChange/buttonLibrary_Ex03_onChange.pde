/*
 * Button Library Example 3 - onChange
 * ------------
 *
 * Circuit: 
 *          - Button, pulled up externally with 10k resitor on pin 2
 *          - LED on pin 9
 *
 * Behavior: 
 *          When the button is pressed and when the button is released
 *          The LED will dim. Behavior loops.
 *
 * Created 13 April 2007
 * Updated 29 October 2010
 * by Carlyn Maw
 *
 */
 
#include <Button.h>

int pwmLED = 9;
int dimFactor = 1;

//Instantiate Button on digital pin 2
//pressed = ground (i.e. pulled high with external resistor)
Button helloButton = Button(2, LOW);

void setup()
{
  pinMode(pwmLED, OUTPUT);      // sets the digital pin as output
}

void loop()
{
  helloButton.listen();
  
 if (helloButton.onChange()){
   if (dimFactor < 255) {
    dimFactor += dimFactor; 
  } else {
    dimFactor = 1; 
  }
 }
  
  analogWrite(pwmLED, 255/dimFactor);     
  


}
