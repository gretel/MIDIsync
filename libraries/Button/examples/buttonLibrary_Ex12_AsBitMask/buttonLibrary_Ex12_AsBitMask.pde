 /*
 * Button Library Example 12 -  hint - really just a bit mask...
 * ------------
 *
 * Circuit: 
 *          - Potentiometer on pin 1
 *          - Extra LED on pin 9
 *
 * Behavior: 
 *          When the button is pressed the LED on pin "ledPin1" will 
 *          be on and the led on "ledPin2" will be off. 
 *
 * Created 13 April 2007 & 25 Jan, 2007
 * Updated 29 October 2010
 * by Carlyn Maw
 *
 */
#include <Button.h>

int potPin = 1;    // select the input pin for the potentiometer
int ledPin1 = 9;   // select the pin for the LED
int ledPin2 = 13;   // select the pin for the LED
int val = 0;       // variable to store the value coming from the sensor

byte sensorAsByte;  //the byte that will be passed to the Button
byte readRegisterValue; 


//Instantiate Button on digital pin 2
//pressed = the bit being read is 1
//pass it a byte by reference where that data is going to live
Button helloButton = Button(3, HIGH, &sensorAsByte);

void setup() {

  pinMode(ledPin1, OUTPUT);  // declare the ledPin as an OUTPUT
  pinMode(ledPin2, OUTPUT);  // declare the ledPin as an OUTPUT
  Serial.begin(9600);
}

void loop() {
 //I used an analog read b/c didn't have shiftRegs at home...
  val = analogRead(potPin);    // read the value from the sensor
  sensorAsByte = val/4;        //make it a byte
  
  helloButton.listen();
  readRegisterValue = helloButton.getRegisterValue();
 
  Serial.print("What I say: ");
  Serial.println(sensorAsByte, BIN);
  Serial.print("What you say: ");
  Serial.println(readRegisterValue, BIN);
  


if (helloButton.isReleased()) {
  digitalWrite(ledPin2, HIGH);
  digitalWrite(ledPin1, LOW);

}

if (helloButton.isPressed()) {
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, LOW);
}

}
