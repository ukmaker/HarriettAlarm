#ifndef HARRIETT_ALARM_H
#define HARRIETT_ALARM_H

#include "ClockTime.h"

class Alarm {

  public:
  Alarm(uint8_t h, uint8_t m, uint8_t tone, uint8_t volume, uint8_t* bitmapBits, uint16_t width, uint16_t height) : 
  _time(h, m),
  _tone(tone),
  _volume(volume),
  _bitmap(bitmapBits, width, height) {}

  ClockTime &time() {
    return _time;
  }

  uint8_t tone() {
    return _tone;
  }

  uint8_t volume() {
    return _volume;
  }

  uint8_t nextTone() {
    _tone++;
    if(_tone > 8) _tone = 1;
    return _tone;
  }

  uint8_t prevTone() {
    if(_tone > 1) {
      _tone--;
    } else {
      _tone = 8;
    }
    return _tone;
  }

  uint8_t louder() {
    _volume += 1;
    if(_volume > 10) _volume = 10;
    return _volume;
  }

  uint8_t quieter() {
    uint8_t dec = 1;
    if(_volume > dec) {
      _volume -= dec;
    } else {
      _volume = 0;
    }
    return _volume;
  }

  Bitmap &bitmap() {
    return _bitmap;
  }

  protected:

  ClockTime _time;
  uint8_t _tone = 1;
  uint8_t _volume = 5;

  Bitmap _bitmap;

};

#endif