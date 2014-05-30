This is an example C++ library for Arduino 0018+, by Carlyn Maw

Installation
--------------------------------------------------------------------------------

To install this library, just place this entire folder as a subfolder in your
~/Documents/Arduino/libraries folder for mac My Documents\Arduino\libraries\ for a
PC.

When installed, this library should look like:

Arduino/libraries/Button              (this library's folder)
Arduino/libraries/Button/Button.cpp     (the library implementation file)
Arduino/libraries/Button/Button.h       (the library description file)
Arduino/libraries/Button/keywords.txt (the syntax coloring file)
Arduino/libraries/Button/examples     (the examples in the "open" menu)
Arduino/libraries/Button/readme.txt   (this file)

Building
--------------------------------------------------------------------------------

After this library is installed, you just have to start the Arduino application.
You may see a few warning messages as it's built.

To use this library in a sketch, go to the Sketch | Import Library menu and
select dLED.  This will add a corresponding line to the top of your sketch:
#include <Button.h>

To stop using this library, delete that line from your sketch.

Geeky information:
After a successful build of this library, a new file named "Button.o" will appear
in "Arduino/libraries/Button". This file is the built/compiled library
code.

If you choose to modify the code for this library (i.e. "Button.cpp" or "Button.h"),
then you must first 'unbuild' this library by deleting the "Button.o" file. The
new "Button.o" with your code will appear after the next press of "verify"

Public Functions
--------------------------------------------------------------------------------

Button myInstance = Button(int pinNumber);

Button myInstance = Button(int pinNumber, bool pressedState);

	myInstance = some name
	pinNumber = some int representing what pin the LED is attached to
	pressedState = is the button HIGH or LOW when pressed?
	
	BEHAVIOR: creates instance of a Button
	
Button myInstance = Button(int pinNumber, bool pressedState, unsigned char *myRegister);

	myInstance = some name
	pinNumber = some int representing what bit the button information is on
	pressedState = is the button HIGH or LOW when pressed?
	*myRegister = the byte containing the button information passed by reference ( &byteName ) 
	
	BEHAVIOR: creates instance of a Button	

    
myInstance.version();
    
    BEHAVIOR: returns a char* of the version	

myInstance.getRegisterValue();
    
    BEHAVIOR: returns a unsigned byte value representing the current value of *myRegister
    
myInstance.listen();

    BEHAVIOR: Does all the work of the function, actually tracks the behavior. Should be called
    at the top of the main loop.
    
myInstance.isReleased();
myInstance.isReleased(bool refreshPinData);


    refreshPinData = value should be set to one if and only if there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen.

    BEHAVIOR: returns true if the debounced value for button is unpressed.
    
myInstance.isPressed();
myInstance.isPressed(bool refreshPinData);

    refreshPinData = value should be set to one if and only if there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 
    
    BEHAVIOR: returns true if the debounced value for button is pressed.

    
myInstance.onChange();
myInstance.onChange(bool refreshPinData); 

    refreshPinData = value should be set to one if and _only if_ there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 
    
    BEHAVIOR: returns true if the debounced value for button has changed.

myInstance.onPress();
myInstance.onPress(bool refreshPinData);

    refreshPinData = value should be set to one if and _only if_ there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 
    
    BEHAVIOR: returns true if the debounced value for button has changed from released to pressed
    
myInstance.onRelease();
myInstance.onRelease(bool refreshPinData);

    refreshPinData = value should be set to one if and _only if_ there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 
    
    BEHAVIOR: returns true if the debounced value for button has changed from pressed to released

        
myInstance.getDebounceDelay();
    
    BEHAVIOR: returns the int vale of the current debounce delay. default is 30 millis.
    
myInstance.setDebounceDelay(unsigned int newDebounceDelay);

    newDebounceDelay = some int for the new value of the debounce delay
    
    BEHAVIOR: updates the debounce delay to be the value of newDebounceDelay in millis

myInstance.clearDebounceDelay();

    BEHAVIOR: updates the debounce delay to be the value of 0. As if there was no debounce.
    
myInstance.onDoubleClick();
myInstance.onDoubleClick(bool refreshPinData);

    refreshPinData = value should be set to one if and _only if_ there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 

    BEHAVIOR: returns true if the debounced value for button has changed from pressed to released
    twice in an amount of time determined by _doubleClickDelay
    
myInstance.getDoubleClickDelay();

    BEHAVIOR: returns the unsigned int vale of the current debounce delay. default is 30 millis.

myInstance.setDoubleClickDelay(unsigned int myDelay);

    myDelay = some unisgned int for the new value of the double click detection window
    
    BEHAVIOR: updates the double click detection window value to that of myDelay in millis
    
myInstance.onPressAsToggle();
myInstance.onPressAsToggle(bool refreshPinData);

    refreshPinData = value should be set to one if and _only if_ there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 
    
    BEHAVIOR: alternates returning true and false every time the button detects an onPress event

myInstance.onReleaseAsToggle();
myInstance.onReleaseAsToggle(bool refreshPinData);

    refreshPinData = value should be set to one if and _only if_ there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 
    
    BEHAVIOR: alternates returning true and false every time the button detects an onRelease event
     
myInstance.isHold();
myInstance.isHold(bool refreshPinData);

    refreshPinData = value should be set to one if and _only if_ there has been A SIGNIFICANT DELAY
    between calling this function and the last time listen was called. Forces a new listen. 
    
    BEHAVIOR: returns true if the button has been held for a period of time as set by the _holdDelay
    
myInstance.getHoldDelay();

    BEHAVIOR: returns the unsigned int value of the current _holdDelay. default is 1500 millis.
    
myInstance.setHoldDelay(unsigned int myDelay);

    myDelay = some unisgned int for the new value of the double click detection window
    
    BEHAVIOR: updates the value of the current _holdDelay.
    
myInstance.getPressCount();

    BEHAVIOR: returns the unsigned int value of the number of button presses. 

myInstance.clearPressCount();  

    BEHAVIOR: resets the value returned by getPressCount() to 0 

myInstance.getReleaseCount();

    BEHAVIOR: returns the unsigned int value of the number of button releases. 

myInstance.clearReleaseCount();

    BEHAVIOR: resets the value returned by getReleaseCount() to 0 
    
myInstance.isNthPress(unsigned int moduloByMe);

    moduloByMe = some number that is important to you
    
    BEHAVIOR: returns true after every Nth onPress (as determined by moduloByMe) until next onPress

myInstance.isNthRelease(unsigned int moduloByMe);

    moduloByMe = some number that is important to you
    
    BEHAVIOR: returns true after every Nth onRelease (as determined by moduloByMe) until next onRelease


	
	
	
	



