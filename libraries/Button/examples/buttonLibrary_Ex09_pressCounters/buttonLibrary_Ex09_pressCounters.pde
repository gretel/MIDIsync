/*
 * Button Library Example 9 -  pressCounter and releaseCounter
 * ------------
 *
 * Circuit: 
 *          - Button, pulled up externally with 10k resitor on pin 2
 *          - Extra LED on pin 9
 *
 * Behavior: 
 *          When the button is pressed the LED will get a little 
 *          bit brighter every time until it loops around.
 *
 *          Every N'th time the button is pressed (N determined
 *          supriseClickNum) the suprise LED is on and a message 
 *          prints to the serial monitor. 
 *
 * Created 13 April 2007
 * Updated 29 October 2010
 * by Carlyn Maw
 *
 */

#include <Button.h>

int pwmLED = 9;
int lightFactor = 10;
byte myBrightness = 0;

int supriseLED = 13;
int supriseClickNum = 7;


//Instantiate Button on digital pin 2
//pressed = ground (i.e. pulled high with external resistor)
Button helloButton = Button(2, LOW);
unsigned int myPressCount = 0;

void setup()
{

  pinMode(pwmLED, OUTPUT);      // sets the digital pin as output
  pinMode(supriseLED, OUTPUT);      // sets the digital pin as output

  Serial.begin(9600);

  Serial.println("When ever you're ready...");



}

void loop()
{
  helloButton.listen();


  if (helloButton.onPress()){
    if (myBrightness == 255) helloButton.clearPressCount();
    myPressCount= helloButton.getPressCount();
    myBrightness = constrain((myPressCount*lightFactor), 0, 255);
    Serial.println(myBrightness, DEC);
  }

  analogWrite(pwmLED, myBrightness); 


  if  (helloButton.getReleaseCount()) {
    if (!(helloButton.getReleaseCount() % supriseClickNum)) {
      digitalWrite(supriseLED, HIGH);
      Serial.println("SUPRISE!!");
    } 
    else {
      digitalWrite(supriseLED, LOW);
    }
  }


}








