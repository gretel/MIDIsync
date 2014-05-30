/*
	Button.h - - Button library for Wiring/Arduino - Version 0.2
	
	Original library 		(0.2) by Carlyn Maw.
	
 */

// ensure this library description is only included once
#ifndef Button_h
#define Button_h
#define LIBRARY_VERSION	 0.2

// include core Wiring API and now Arduino
#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// library interface description
class Button {
 
  // user-accessible "public" interface
  public:
  // constructors:
    Button(int myPin, bool myMode);
    Button(int myBit, bool myMode, unsigned char *myRegister);
    
    char* version(void);			// get the library version
    unsigned char getRegisterValue(void);
    
    void listen(void);
    
    bool isReleased(void);
    bool isPressed(void);
    bool isReleased(bool refreshPinData);
    bool isPressed(bool refreshPinData);
    
    bool onChange(void);  
    bool onPress(void);
    bool onRelease(void);
    bool onChange(bool refreshPinData);  
    bool onPress(bool refreshPinData);
    bool onRelease(bool refreshPinData);
        
    unsigned int getDebounceDelay(void);
    void setDebounceDelay(unsigned int);
    void clearDebounceDelay(void);
    
    bool onDoubleClick(void);
    bool onDoubleClick(bool refreshPinData);
    
    unsigned int getDoubleClickDelay(void);
    void setDoubleClickDelay(unsigned int);
    
    bool onPressAsToggle(void);
    bool onReleaseAsToggle(void);
    bool onPressAsToggle(bool refreshPinData);
    bool onReleaseAsToggle(bool refreshPinData);
     
    bool isHold(void);
    bool isHold(bool refreshPinData);
    
    unsigned int getHoldDelay(void);
    void setHoldDelay(unsigned int);
    
    unsigned int getPressCount(void);
    void clearPressCount(void);    
    unsigned int getReleaseCount(void);
    void clearReleaseCount(void);
    
    bool isNthPress(unsigned int moduloByMe);
    bool isNthRelease(unsigned int moduloByMe);
    
 


  // library-accessible "private" interface
  private:
    int _myPin;
    int _myBit;
    unsigned char *_myRegister;
    unsigned char _registerValue;
    bool _type;  //direct pin or shift register
    bool _mode;  //HIGH == pressed (1) or LOW == pressed (0)
    
    bool _lastState;
    bool _currentState;
    
    bool _debounced;
    bool _lastDebouncedState;
    bool _currentDebouncedState;
    unsigned long int _debounceTimerStartTime;
    unsigned int _debounceDelay;
    
    bool _pressed;
    bool _released;
    
    bool _changed;
    bool _justPressed;
    bool _justReleased;
    unsigned int _pressCount;
    unsigned int _releaseCount;
   
    
    unsigned int _doubleClickDelay;
    unsigned int _holdDelay;
    
    bool _pToggleFlag;
    bool _rToggleFlag;
    
    unsigned long int _lastPressTime;
    unsigned long int _currentPressTime;
    unsigned long int _lastReleaseTime;
    unsigned long int _currentReleaseTime;
    
    unsigned long int _currentTime;

  
};

#endif

