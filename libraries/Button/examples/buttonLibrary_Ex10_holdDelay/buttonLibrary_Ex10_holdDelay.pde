 /*
 * Button Library Example 9 -  Hold Delay
 * ------------
 *
 * Circuit: 
 *          - Button, pulled up externally with 10k resitor on pin 2
 *          - Extra LED on pin 9
 *
 * Behavior: 
 *         LED is on after a hold delay.
 *
 *         The startup prints out the default value of the 
 *         hold delay default (1500 millis) and then 
 *         updates it with the value in myHoldDelay
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
int myHoldDelay = 3000;         //1500 is the default

void setup()
{

  pinMode(ledPin1, OUTPUT);      // sets the digital pin as output
  pinMode(ledPin2, OUTPUT);      // sets the digital pin as output
  
  Serial.begin(9600);
  Serial.print("Default = \t");
  Serial.println(helloButton.getHoldDelay());
  helloButton.setHoldDelay(myHoldDelay);
  Serial.print("New Delay = \t");
  Serial.println(helloButton.getHoldDelay());
  
  
}

void loop()
{
  helloButton.listen();
  
 if (helloButton.isHold()){
    digitalWrite(ledPin1, LOW);
    digitalWrite(ledPin2, HIGH);
   
 } else {
    digitalWrite(ledPin2, LOW);
    digitalWrite(ledPin1, HIGH);
 }
 
  


}
