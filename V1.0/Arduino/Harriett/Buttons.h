#ifndef HARRIETT_BUTTONS_H
#define HARRIETT_BUTTONS_H

#include "Button.h"

typedef void (*EventHandler)(void);
/**
* Encapsulates the two by two buttons and raises appropriate events when they are pressed
**/
class Buttons {

  public:

  Buttons(uint32_t b1, uint32_t b2, uint32_t b3, uint32_t b4) :
    _b1(Button(b1)), _b2(Button(b2)), _b3(Button(b3)), _b4(Button(b4))
    {

    }

  bool active() {
    return _active;
  }

  void update() {
    _active = 0;
    _b1.update();
    _b2.update();
    _b3.update();
    _b4.update();
    // let's figure out what was clicked
    if (_b1.isActive() || _b2.isActive()|| _b3.isActive() || _b4.isActive()) {
      // something's happening
      //set the flag so the shell can inhibit sleeping for a while
      _active = 1;
    }

    if(_b1.isClicked() && _b2.isNotActive()) {
      raiseEvent(_backLeftClickHandler);
      return;
    }
    
    if(_b2.isClicked() && _b1.isNotActive()) {
      raiseEvent(_backRightClickHandler);
      return;
    }
    
    if (_b1.isLongClicked() && _b2.isPressed()) {
      raiseEvent(_backBothLongClickHandler);
      return;
    }

    if (_b2.isLongClicked() && _b1.isPressed()) {
      raiseEvent(_backBothLongClickHandler);
      return;
    }

    if (_b1.isLongClicked() && _b2.isNotActive()) {
      raiseEvent(_backLeftLongClickHandler);
      return;
    }

    if (_b2.isLongClicked() && _b1.isNotActive()) {
      raiseEvent(_backRightLongClickHandler);
      return;
    }

    if (_b3.isClicked()) {
      raiseEvent(_frontRightClickHandler);
      return;
    }

    if (_b3.isLongPressed() || _b3.isRepeating()) {
      raiseEvent(_frontLeftRepeatClickHandler);
    }

    if (_b4.isClicked()) {
      raiseEvent(_frontLeftClickHandler);
      return;
    }

    if (_b4.isLongPressed() || _b4.isRepeating()) {
      raiseEvent(_frontRightRepeatClickHandler);
    }
  }

  void setBackLeftLongClickHandler(EventHandler h) {
    _backLeftLongClickHandler = h;
  }
  void setBackRightLongClickHandler(EventHandler h) {
    _backRightLongClickHandler = h;
  }
  void setBackBothLongClickHandler(EventHandler h) {
    _backBothLongClickHandler = h;
  }

  void setBackLeftClickHandler(EventHandler h) {
    _backLeftClickHandler = h;
  }

  void setBackRightClickHandler(EventHandler h) {
    _backRightClickHandler = h;
  }

  void setFrontRightClickHandler(EventHandler h) {
    _frontRightClickHandler = h;
  }
  void setFrontLeftClickHandler(EventHandler h) {
    _frontLeftClickHandler = h;
  }

  void setFrontleftRepeatClickHandler(EventHandler h) {
    _frontLeftRepeatClickHandler = h;
  }
  void setFrontRightRepeatClickHandler(EventHandler h) {
    _frontRightRepeatClickHandler = h;
  }

protected:
    Button _b1;
    Button _b2;
    Button _b3;
    Button _b4;

  void raiseEvent(EventHandler h) {
    if (h != NULL) {
      h();
    }
  }
  bool _active = 0;
  uint8_t _down = 0;
  EventHandler _backLeftLongClickHandler = NULL;
  EventHandler _backRightLongClickHandler = NULL;
  EventHandler _backBothLongClickHandler = NULL;
  EventHandler _frontRightClickHandler = NULL;
  EventHandler _frontLeftClickHandler = NULL;
  EventHandler _frontLeftRepeatClickHandler = NULL;
  EventHandler _frontRightRepeatClickHandler = NULL;
  EventHandler _backRightClickHandler = NULL;
  EventHandler _backLeftClickHandler = NULL;
};
#endif