/*
  Button.h - Button library for Wiring/Arduino
  (cc) 2007 Carlyn Maw .  Some rights reserved.
  
  Created  2007 Apr 13 
  Updated 2012 Apr 5
  Version 0.2
*/

// include this library's description file
#include "Button.h"

   
  
// Constructor /////////////////////////////////////////////////////////////////
// Function that handles the creation and setup of instances

//------------------------------------------------------ Using Arduino Pin Num
Button::Button(int myPin, bool myMode)
{
    // initialize this instance's variables
    this->_myPin = myPin;
    
    pinMode(this->_myPin, INPUT);
    
    this->_type = 0;
    this->_myBit = this->_myPin;
    this->_mode = myMode;
    
    this->_registerValue = 0;
    
    this->_lastState = 0;
    this->_currentState = 0;
    
    this-> _debounced = 1;
    this-> _lastDebouncedState = 0;
    this-> _currentDebouncedState  = 0;
    this-> _debounceTimerStartTime = 0;
    this-> _debounceDelay = 30;
    
    this-> _pressed = 0;
    this-> _released = 1;
  
    this-> _changed = 0;
    this-> _justPressed = 0;
    this-> _justReleased = 0;
    this-> _pressCount = 0;
    this-> _releaseCount = 0;
    
    this-> _doubleClickDelay = 400; 
    this-> _holdDelay = 1500; 
    
    this-> _pToggleFlag = 0; //starts in off
    this-> _rToggleFlag = 1; //starts in off
    
    this-> _lastPressTime = 0;
    this-> _currentPressTime = 0;
    this-> _lastReleaseTime = 0;
    this-> _currentReleaseTime = 0;
    
  
}

//----------------------------------------------------------------- Using Byte
Button::Button(int myBit, bool myMode, unsigned char *myRegister)
{
    // initialize this instance's variables
    this->_type = 1;
    this->_mode = myMode;
    this->_myBit = myBit;
    this->_myPin = this->_myBit;  
    this->_myRegister = myRegister;
    
    this->_registerValue = 255;
    
    this->_lastState = 0;
    this->_currentState = 0;
    
    this-> _debounced = 1;
    this-> _lastDebouncedState = 0;
    this-> _currentDebouncedState = 0;
    this-> _debounceTimerStartTime = 0;
    this-> _debounceDelay = 30;
    
    this-> _pressed = 0;
    this-> _released = 1;
  
    this-> _changed = 0;
    this-> _justPressed = 0;
    this-> _justReleased = 0;
    this-> _pressCount = 0;
    this-> _releaseCount = 0;
    
    this-> _doubleClickDelay = 400; 
    this-> _holdDelay = 1500; 
    
    this-> _pToggleFlag = 0; //starts in off
    this-> _rToggleFlag = 1; //starts in off
    
    this-> _lastPressTime = 0;
    this-> _currentPressTime = 0;
    this-> _lastReleaseTime = 0;
    this-> _currentReleaseTime = 0;
  
  
}

// Public Methods //////////////////////////////////////////////////////////////
// Functions available in Wiring sketches, this library, and other libraries

//---------////////////////////MAIN LOOP / LISTENER ///////////--------------//

  void Button::listen(void){
  
  _currentTime = millis();
  
  if(!_type) {
  	this->_currentState = digitalRead(this->_myPin);
  } else {
    _registerValue = *_myRegister;
  	this->_currentState = _registerValue & (1 << _myBit);
  }
  
  if (_currentState != _lastState) {
    _debounced = false;
    _debounceTimerStartTime = _currentTime;
  } else if ((_currentTime - _debounceTimerStartTime) > _debounceDelay) {
    _debounced = true;
  }
  
  if (_debounced) {
    _lastDebouncedState = _currentDebouncedState;
    _currentDebouncedState = _currentState;
  }
  
  
  if (_currentDebouncedState == _mode) {
    _pressed = true;
    _released = false;
    _justReleased = false;
  } else {
    _pressed = false;
    _released = true;
    _justPressed = false;
  }
  
  
  if (_lastDebouncedState != _currentDebouncedState) {
    _changed = true;
  } else {
    _changed = false;
    _justPressed = false;
    _justReleased = false;
  }
  
  if (_changed && _pressed) {
    _justPressed = true;
    _pressCount ++;
    _lastPressTime = _currentPressTime;
    _currentPressTime = _currentTime;
    _justReleased = false;
  } else if(_changed && _released){
    _justPressed = false;
    _justReleased = true;
    _releaseCount ++;
    _lastReleaseTime = _currentReleaseTime;
    _currentReleaseTime = _currentTime;
  } else if(!_changed) { //added no effect...
    _justPressed = false; //added no effect...
    _justReleased = false; //added no effect...
  } else if (_released) { //added no effect...
      _justPressed = false; //added no effect...
  } else if (_pressed) { //added no effect...
    _justReleased = false; //added no effect...
  }
  	
  //wrap up the funtion
  _lastState = _currentState;
    
  }
  
 //----------------------------------------------------------- END LISTENER
 
 // -------------------------------------------------------//// DEBOUNCE /// 
 
unsigned int Button::getDebounceDelay(void) {
	return _debounceDelay;
}

void Button::setDebounceDelay(unsigned int newDebounceDelay) {
	_debounceDelay = newDebounceDelay;
}

 void Button::clearDebounceDelay(void){
 	setDebounceDelay(0);
 }
 
 
 
  // -------------------------------------------------------//// UP OR DOWN /// 
  
 //----------------------------------------------------- isReleased
 bool Button::isReleased(bool refreshPinData){
  
     if (refreshPinData) {
  		listen();
  		return _released;
  	} else {
  		return _released;
	}
  }

 //------------------------------------------------- overload
 bool Button::isReleased(void){
  		return isReleased(0);
  }
  
 //----------------------------------------------------- isPressed
 bool Button::isPressed(bool refreshPinData){
   if (refreshPinData) {
  		listen();
  		return _pressed; 
  	} else {
  		return _pressed; 
  	}
  }
  
  
 //------------------------------------------------ overload
  bool Button::isPressed(void){
  		return isPressed(0); 
  }
  
  
     // -------------------------------------------------------//// COUNTS /// 
 
unsigned int Button::getPressCount(void) {
	return _pressCount;
}

 void Button::clearPressCount(void){
 	_pressCount = 0;
 }
 
unsigned int Button::getReleaseCount(void) {
	return _pressCount;
}

 void Button::clearReleaseCount(void){
 	_releaseCount = 0;
 } 
  
  
  // -------------------------------------------------------////   EVENTS   /// 
  
 
 //---------------------------------------------------- onChange
 bool Button::onChange(void){
    return _changed;
  }
   
 //------------------------------------------------ overload  
 bool Button::onChange(bool refreshPinData){
    if (refreshPinData) {
  		listen();
  		return _changed; 
  	} else {
  		return _changed; 
  	}
  }
  
 //---------------------------------------------------- onPress 
  bool Button::onPress(void){
    return _justPressed;

  }
 //------------------------------------------------ overload
   bool Button::onPress(bool refreshPinData){
   if (refreshPinData) {
  		listen();
  		return onPress();  
  	} else {
  		return onPress(); 
  	}

  }
    
 //---------------------------------------------------- onRelease
  bool Button::onRelease(void){
    return _justReleased;

  }
 //------------------------------------------------ overload 
    bool Button::onRelease(bool refreshPinData){
   if (refreshPinData) {
  		listen();
  		return onRelease(); 
  	} else {
  		return onRelease();  
  	}

  }

 //---------------------------------------------------- TOGGLES 
bool Button::onPressAsToggle(void){
   if (_justPressed) {
    _pToggleFlag ? _pToggleFlag=false : _pToggleFlag=true;
  }
  return _pToggleFlag;
 }
 
 
bool Button::onPressAsToggle(bool refreshPinData){
    if (refreshPinData) {
        listen();    
        return onPressAsToggle();
    }
    else {
        return(onPressAsToggle());
    }
}
 
bool Button::onReleaseAsToggle(){
   if (_justReleased) {
    _rToggleFlag ? _rToggleFlag=false : _rToggleFlag=true;
  }
  return _rToggleFlag;
 }
 
bool Button::onReleaseAsToggle(bool refreshPinData){
    if (refreshPinData) {
        listen();    
        return onReleaseAsToggle();
    }
    else {
        return onReleaseAsToggle();
    }
}

 //------------------------------------------------------- DOUBLE CLICK
 //----------------------------------------------------- OnDoubleClick
 
bool Button::onDoubleClick(void)  {

    return(_justReleased && (_currentReleaseTime - _lastReleaseTime) <= _doubleClickDelay);

}
 //--------------------------------------------------- overload
bool Button::onDoubleClick(bool refreshPinData){
    if (refreshPinData) {
        listen();    
        return onDoubleClick();
    }
    else {
        return onDoubleClick();
    }
}

   // --------------------------------        --------- get and set delay 
 
unsigned int Button::getDoubleClickDelay(void) {
	return _doubleClickDelay;
}

void Button::setDoubleClickDelay(unsigned int newDoubleClickDelay) {
	_doubleClickDelay = newDoubleClickDelay;
}

 //---------------------------------------------------------------- HOLDING
 //----------------------------------------------------- OnHold
 
bool Button::isHold(void)  {

    return _pressed && (_currentTime - _currentPressTime) > _holdDelay;

}
 //--------------------------------------------------- overload
bool Button::isHold(bool refreshPinData){
    if (refreshPinData) {
        listen();    
        return isHold();
    }
    else {
        return isHold();
    }
}

   // ----------------------------------------- get and set delay 
 
unsigned int Button::getHoldDelay(void) {
	return _holdDelay;
}

void Button::setHoldDelay(unsigned int newHoldDelay) {
	_holdDelay = newHoldDelay;
}

 //----------------------------------------------------------- isNTH Sillyness

bool Button::isNthPress(unsigned int moduloByMe) {
  if  (_pressCount) {
    if (!(_pressCount % moduloByMe)) {
        return true;
    } 
    else {
      return false;
    }
  }
  }


bool Button::isNthRelease(unsigned int moduloByMe) {
  if  (_releaseCount) {
    if (!(_releaseCount % moduloByMe)) {
      return true;
    } 
    else {
      return false;
    }
  }
  }
  

// -------------------------------------------------------////    ADMIN   /// 
unsigned char Button::getRegisterValue(void){
	return(_registerValue);
}


/*
	version() returns the version of the library:
*/
char* Button::version(void) {
	return "0.1";
}