#ifndef HARRIETT_ALARM_H
#define HARRIETT_ALARM_H

#include "ClockTime.h"

#define ALARM_MAX_TONES 2

class Alarm {

  public:
  Alarm(uint8_t h, uint8_t m, uint8_t tone, uint8_t volume, uint8_t* bitmapBits, uint16_t width, uint16_t height) : 
  _time(h, m),
  _bitmap(bitmapBits, width, height) {
    setVolume(volume);
    setTone(tone);
  }

  ClockTime &time() {
    return _time;
  }

  uint8_t tone() {
    return _tone;
  }

  uint8_t volume() {
    return _volume;
  }

  void setVolume(uint8_t vol) {
    _volume = vol;
    if(_volume > 10) _volume = 10;
  }

  void setTone(uint8_t tone) {
    if(_tone > ALARM_MAX_TONES) {
      _tone = ALARM_MAX_TONES;
    } else {
      _tone = tone;
    }  
  }

  uint8_t nextTone() {
    _tone++;
    if(_tone > ALARM_MAX_TONES) _tone = ALARM_MAX_TONES;
    return _tone;
  }

  uint8_t prevTone() {
    if(_tone > 1) {
      _tone--;
    }
    return _tone;
  }

  uint8_t louder() {
    if(_volume < 10) {
      _volume += 1;
    }
    return _volume;
  }

  uint8_t quieter() {
    if(_volume > 1) {
      _volume -= 1;
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