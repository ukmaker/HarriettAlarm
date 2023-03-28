#ifndef UKMAKER_TWOBYTWO_H
#define UKMAKER_TWOBYTWO_H

#include "Button.h"


class TwoByTwo {

  public:

  TwoByTwo(uint32_t r, uint32_t g, uint32_t b, 
  uint32_t b1, uint32_t b1l, 
  uint32_t b2, uint32_t b2l, 
  uint32_t b3, uint32_t b3l, 
  uint32_t b4, uint32_t b4l) :
    _r(r), _g(g), _b(b), _b1(Button(b1)), _b2(Button(b2)), _b3(Button(b3)), _b4(Button(b4)),
    _b1l(b1l), _b2l(b2l), _b3l(b3l), _b4l(b4l)
    {
      pinMode(_r, OUTPUT);
      digitalWrite(_r, HIGH);

      pinMode(_g, OUTPUT);
      digitalWrite(_g, HIGH);

      pinMode(_b, OUTPUT);
      digitalWrite(_b, HIGH);

      pinMode(_b1l, OUTPUT);
      digitalWrite(_b1l, HIGH);

      pinMode(_b2l, OUTPUT);
      digitalWrite(_b2l, HIGH);

      pinMode(_b3l, OUTPUT);
      digitalWrite(_b3l, HIGH);

      pinMode(_b4l, OUTPUT);
      digitalWrite(_b4l, HIGH);

    }

  void update() {
    _b1.update();
    _b2.update();
    _b3.update();
    _b4.update();
  }

  Button &b1() { return _b1; }
  Button &b2() { return _b2; }
  Button &b3() { return _b3; }
  Button &b4() { return _b4; }

  protected:
    Button _b1;
    Button _b2;
    Button _b3;
    Button _b4;
    uint32_t _r, _g, _b;
    uint32_t _b1l, _b2l, _b3l, _b4l;
};

#endif