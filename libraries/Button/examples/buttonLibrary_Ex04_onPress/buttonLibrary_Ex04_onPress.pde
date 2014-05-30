 /*
 * Button Library Example 4 -  onPress
 * ------------
 *
 * Circuit: 
 *          - Button, pulled up externally with 10k resitor on pin 2
 *          - LED on pin 9
 *
 * Behavior: 
 *          When the button is pressed The LED will dim a little more.
 *          Behavior loops.
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
  Serial.begin(9600);
}

void loop()
{
  helloButton.listen();
  
 if (helloButton.onPress()){
  if (dimFactor < 255) {
    dimFactor += dimFactor; 
  } else {
    dimFactor = 1; 
  }
   
 }
  
  analogWrite(pwmLED, 255/dimFactor);     
  


}
