#ifndef HARRIETT_BUTTONS_H
#define HARRIETT_BUTTONS_H

#include "TwoByTwo.h"
typedef void (*EventHandler)(void);
/**
* Encapsulates the two by two buttons and raises appropriate events when they are pressed
**/
class Buttons {

  public:

  Buttons(TwoByTwo &tbt)
    : _tbt(tbt) {}

  bool active() {
    return _active;
  }

  void update() {
    _active = 0;
    _tbt.update();
    // let's figure out what was clicked
    if (_tbt.b1().isActive() || _tbt.b2().isActive()|| _tbt.b3().isActive() || _tbt.b4().isActive()) {
      // something's happening
      //set the flag so the shell can inhibit sleeping for a while
      _active = 1;
    }

    // If one of the work or lesson buttons is held down to produce a long click
    // then we need to raise an event to toggle that alarm being active
    // there should be only one event per active session (i.e. ignore short click and repeat)
    // But if the second button is pushed while waiting for the first long click, we need to produce a set click
    // which raises an event to say we are setting the time on whatever is active
    // i.e. one of the alarms or the actual time itself

    if(_tbt.b1().isClicked() && _tbt.b2().isNotActive()) {
      raiseEvent(_backLeftClickHandler);
      return;
    }
    
    if(_tbt.b2().isClicked() && _tbt.b1().isNotActive()) {
      raiseEvent(_backRightClickHandler);
      return;
    }
    
    if (_tbt.b1().isLongClicked() && _tbt.b2().isPressed()) {
      raiseEvent(_setTimeClickHandler);
      return;
    }

    if (_tbt.b2().isLongClicked() && _tbt.b1().isPressed()) {
      raiseEvent(_setTimeClickHandler);
      return;
    }

    if (_tbt.b1().isLongClicked() && _tbt.b2().isNotActive()) {
      raiseEvent(_workLongClickHandler);
      return;
    }

    if (_tbt.b2().isLongClicked() && _tbt.b1().isNotActive()) {
      raiseEvent(_lessonLongClickHandler);
      return;
    }
    // that's it for those buttons - ignore any repeat clicks

    // Now for the snooze/stop buttons
    // Pressing both does nothing
    // Pressing either alone produces repeating clicks
    // It's up to the controller to determine what that means according to context
    if (_tbt.b3().isClicked()) {
      raiseEvent(_snoozeClickHandler);
      return;
    }

    if (_tbt.b3().isLongPressed() || _tbt.b3().isRepeating()) {
      raiseEvent(_snoozeRepeatClickHandler);
    }

    if (_tbt.b4().isClicked()) {
      raiseEvent(_stopClickHandler);
      return;
    }

    if (_tbt.b4().isLongPressed() || _tbt.b4().isRepeating()) {
      raiseEvent(_stopRepeatClickHandler);
    }
  }

  void setWorkLongClickHandler(EventHandler h) {
    _workLongClickHandler = h;
  }
  void setLessonLongClickHandler(EventHandler h) {
    _lessonLongClickHandler = h;
  }
  void setSetTimeClickHandler(EventHandler h) {
    _setTimeClickHandler = h;
  }

  void setBackLeftClickHandler(EventHandler h) {
    _backLeftClickHandler = h;
  }

  void setBackRightClickHandler(EventHandler h) {
    _backRightClickHandler = h;
  }

  void setSnoozeClickHandler(EventHandler h) {
    _snoozeClickHandler = h;
  }
  void setStopClickHandler(EventHandler h) {
    _stopClickHandler = h;
  }

  void setSnoozeRepeatClickHandler(EventHandler h) {
    _snoozeRepeatClickHandler = h;
  }
  void setStopRepeatClickHandler(EventHandler h) {
    _stopRepeatClickHandler = h;
  }

protected:

  void raiseEvent(EventHandler h) {
    if (h != NULL) {
      h();
    }
  }

  TwoByTwo _tbt;
  bool _active = 0;
  uint8_t _down = 0;
  EventHandler _workLongClickHandler = NULL;
  EventHandler _lessonLongClickHandler = NULL;
  EventHandler _setTimeClickHandler = NULL;
  EventHandler _snoozeClickHandler = NULL;
  EventHandler _stopClickHandler = NULL;
  EventHandler _snoozeRepeatClickHandler = NULL;
  EventHandler _stopRepeatClickHandler = NULL;
  EventHandler _backRightClickHandler = NULL;
  EventHandler _backLeftClickHandler = NULL;
};
#endif