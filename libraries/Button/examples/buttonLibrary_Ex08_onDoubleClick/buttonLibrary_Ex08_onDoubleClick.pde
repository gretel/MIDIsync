 /*
 * Button Library Example 8 -  onDoubleClick
 * ------------
 *
 * Circuit: 
 *          - Button, pulled up externally with 10k resitor on pin 2
 *          - Extra LED on pin 9
 *
 * Behavior: 
 *          When the button is double clicked the LED will dim a 
 *          little more. Behavior loops.
 *
 *         The startup prints out the default value of the 
 *         double click delay default (400 millis) and then 
 *         updates it with the value in myDoubleClickDelay. 
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
int myDoubleClickDelay = 500;       //400 is the default

void setup()
{

  pinMode(pwmLED, OUTPUT);      // sets the digital pin as output
  
  Serial.begin(9600);
  Serial.print("Default = \t");
  Serial.println(helloButton.getDoubleClickDelay());
  helloButton.setDoubleClickDelay(myDoubleClickDelay);
  Serial.print("New Delay = \t");
  Serial.println(helloButton.getDoubleClickDelay());
  
  
}

void loop()
{
  helloButton.listen();
  
 if (helloButton.onDoubleClick()){
  if (dimFactor < 255) {
    dimFactor += dimFactor; 
  } else {
    dimFactor = 1; 
  }
   
 }
  
  analogWrite(pwmLED, 255/dimFactor);     
  


}
