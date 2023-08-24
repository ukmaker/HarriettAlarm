#ifndef HARRIETT_TIME_H
#define HARRIETT_TIME_H

#include <STM32RTC.h>

class ClockTime {

public:

  ClockTime(uint8_t h, uint8_t m)
    : _h(h), _m(m), _s(0) {}

  uint8_t h() {
    return _h;
  }

  uint8_t m() {
    return _m;
  }

  uint8_t s() {
    return _s;
  }

  uint8_t incrementHours(bool byFive) {
    _h = increment(_h, 23, 5, byFive);
    return _h;
  }

  uint8_t incrementMinutes(bool byFive) {
    _m = increment(_m, 59, 5, byFive);
    return _m;
  }

  void getRTC() {
    STM32RTC& rtc = STM32RTC::getInstance();
    uint8_t h, m, s;
    uint32_t ss;
    rtc.getTime(&h, &m, &s, &ss, nullptr);
    _h = h;
    _m = m;
    _s = s;
  }

  void setRTC() {
    STM32RTC& rtc = STM32RTC::getInstance();
    rtc.setHours(_h);
    rtc.setMinutes(_m);
  }

protected:
  uint8_t _h, _m, _s;


  uint8_t increment(uint8_t value, uint8_t limit, uint8_t bump, bool doBump) {
    uint8_t result;
    if (doBump) {
      result = value - (value % bump) + bump;
    } else {
      result = value + 1;
    }

    if (result > limit) {
      result = 0;
    }

    return result;
  }
};
#endif