#ifndef UKMAKER_BUTTON_H
#define UKMAKER_BUTTON_H



#define B_IDLE 0
#define B_FALLEN 1
#define B_LOW 2
#define B_LONG_PRESSED 3
#define B_LONG_LOW 4
#define B_NOISY 5


#define DEBOUNCE_TIME 100
#define LONG_PRESS_TIME 1000
#define REPEAT_DELAY 1000
#define REPEAT_CADENCE 500

class Button {

public:

  Button(uint32_t pin)
    : _pin(pin) {
    pinMode(pin, INPUT_PULLUP);
  }

  ~Button() {}

  void update() {

    uint32_t now = millis();
    bool current_s;

    _repeating = 0;

    current_s = digitalRead(_pin);

    if (_pin_state != current_s) {
      // pin state has changed, reset the debounce counter
      _time = now;
      _state = B_NOISY;
      _pin_state = current_s;
      _next = now + LONG_PRESS_TIME + REPEAT_DELAY;
      _fallen = 0;
      return;
    }

    if(current_s) {
      // Pin is high 
      _state = B_IDLE;
      return;
    }

    // at this point we know the pin has gone low
    if(now > (_time + DEBOUNCE_TIME)) {
      // have we already issued a falling event?
      if(_state == B_NOISY) {
        _state = B_FALLEN;
      } else if(_state == B_FALLEN) {
        _state = B_LOW;
      }
    }

    if(now > (_time + LONG_PRESS_TIME)) {
      if(_state == B_LOW) {
        _state = B_LONG_PRESSED;
      } else { 
        _state = B_LONG_LOW;
      }
      _repeating = 0;
    }

    if(_state == B_LONG_LOW && now > _next) {
      _repeating = 1;
      _next = now + REPEAT_CADENCE;
    }
  }

  bool isRepeating() {
    return _repeating;
  }

  bool isPressed() {
    return _state == B_LOW || _state == B_FALLEN;
  }

  bool isClicked() {
    return _state == B_FALLEN;
  }

  bool isLongPressed() {
    return _state == B_LONG_PRESSED || _state == B_LONG_LOW;
  }

  bool isLongClicked() {
    return _state == B_LONG_PRESSED;
  }

  bool isActive() {
    return !isNotActive();
  }

  bool isNotActive() {
    return _state == B_IDLE;
  }


protected:

  uint32_t _pin;
  uint32_t _time = 0;       // for debouncing
  uint32_t _next;           // time at which the next repeat will be signalled
  uint8_t _state = B_IDLE;  // has the button been processed
  volatile bool _int = 0;
  bool _pin_state = 1;
  bool _repeating = 0;
  bool _fallen = 0;
};

#endif
