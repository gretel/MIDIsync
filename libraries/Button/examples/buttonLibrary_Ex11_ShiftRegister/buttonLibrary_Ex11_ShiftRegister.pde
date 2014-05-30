 /*
 * Button Library Example 11 -  with a shift register
 * ------------
 *
 * Circuit: 
 *          - Button, pulled down externally with 10k resitor
 *            attached to shitregister on its "paralell input" 3
 *          - Extra LED on pin 4
 *          - shift register: 
 *                latchPin = 8;
 *                dataPin = 9;
 *                clockPin = 7;
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

//define where your pins are
int latchPin = 8;
int dataPin = 9;
int clockPin = 7;

int potPin = 1;    // select the input pin for the potentiometer
int ledPin1 = 4;   // select the pin for the LED
int ledPin2 = 13;   // select the pin for the LED
int val = 0;       // variable to store the value coming from the sensor
byte readRegisterValue;  //trouble shooting variable



//Define variables to hold the data for each shift register.
//starting with non-zero numbers can help troubleshoot
byte switchVar1 = B00001000;  //this is true for this button example

//Instantiate Button on digital pin 2
//pressed = VCC (i.e. pulled LOW with external resistor)
//pass it a byte by reference where that data is going to live
Button helloButton = Button(3, HIGH, &switchVar1);

void setup() {
  //start serial
  Serial.begin(9600);

  //define pin modes
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT); 
  pinMode(dataPin, INPUT);
  
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT); 

}

void loop() {

  //Pulse the latch pin:
  //set it to 1 to collect parallel data
  digitalWrite(latchPin,1);
  //set it to 1 to collect parallel data, wait
  delayMicroseconds(20);
  //set it to 0 to transmit data serially  
  digitalWrite(latchPin,0);

  //while the shift register is in serial mode
  //collect each shift register into a byte
 //switchVar1 = shiftIn(dataPin, clockPin);

  //Print out the results.
  //leading 0's at the top of the byte 
  //(7, 6, 5, etc) will be dropped before 
  //the first pin that has a high input
  //reading  
 Serial.print("Shift register reading:\t");
 Serial.println(switchVar1, BIN);



  helloButton.listen();
  readRegisterValue = helloButton.getRegisterValue();
  
  Serial.print("Button library hears:\t");
  Serial.println(readRegisterValue, BIN);
  Serial.println("-------------------");
  

if (helloButton.isReleased()) {
  digitalWrite(ledPin2, HIGH);
  digitalWrite(ledPin1, LOW);

}

if (helloButton.isPressed()) {
  digitalWrite(ledPin1, HIGH);
  digitalWrite(ledPin2, LOW);
}


}

//------------------------------------------------end main loop

////// ----------------------------------------shiftIn function
///// just needs the location of the data pin and the clock pin
///// it returns a byte with each bit in the byte corresponding
///// to a pin on the shift register. leftBit 7 = Pin 7 / Bit 0= Pin 0

byte shiftIn(int myDataPin, int myClockPin) { 
  int i;
  int temp = 0;
  int pinState;
  byte myDataIn = 0;

  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, INPUT);

//we will be holding the clock pin high 8 times (0,..,7) at the
//end of each time through the for loop

//at the begining of each loop when we set the clock low, it will
//be doing the necessary low to high drop to cause the shift
//register's DataPin to change state based on the value
//of the next bit in its serial information flow.
//The register transmits the information about the pins from pin 7 to pin 0
//so that is why our function counts down
  for (i=7; i>=0; i--)
  {
    digitalWrite(myClockPin, 0);
    delayMicroseconds(2);
    temp = digitalRead(myDataPin);
    if (temp) {
      pinState = 1;
      //set the bit to 0 no matter what
      myDataIn = myDataIn | (1 << i);
    }
    else {
      //turn it off -- only necessary for debuging
     //print statement since myDataIn starts as 0
      pinState = 0;
    }

    //Debuging print statements
    //Serial.print(pinState);
    //Serial.print("     ");
    //Serial.println (dataIn, BIN);

    digitalWrite(myClockPin, 1);

  }
  //debuging print statements whitespace
  //Serial.println();
  //Serial.println(myDataIn, BIN);
  return myDataIn;
}

