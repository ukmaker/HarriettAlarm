#ifndef HARRIETT_MP3ALARM_H
#define HARRIETT_MP3ALARM_H

#include "DFRobotDFPlayerMini.h"
#include "SoftwareSerial.h"

static const uint8_t _volumes[] = { 50, 50, 80, 50, 80, 50, 80, 50 };

class MP3Alarm {



public:

  MP3Alarm(uint32_t powerPin, HardwareSerial *serial, uint32_t rxPin, uint32_t txPin) : 
    _mp3(), _powerPin(powerPin),_serial(serial), _rxPin(rxPin), _txPin(txPin)
  {
    pinMode(_powerPin, OUTPUT);
    digitalWrite(_powerPin, LOW);
    _powerOn = 0;
  }

    bool init() {
      powerOn();
      if(!_mp3.begin(*_serial, 1, 1)) {
        return 0;
      }

      _mp3.setTimeOut(500);
      _mp3.outputDevice(DFPLAYER_DEVICE_SD);

      return 1;
    }

/**
* volumeLevel is 1 to 10
**/
  void play(uint8_t sound, uint8_t volumeLevel) {
    powerOn();
    // Acceptable sound values are 1-8
    sound = (sound % 8) + 1;
    // Volume for the MP3 widget is 0 - 30
    // apply the multiplier so that volumePercent == 50 is the same audible volume for all sounds
    if(volumeLevel > 10) {
      volumeLevel = 10;
    }
    uint32_t v = ((volumeLevel * 30) / 10) * _volumes[sound] / 100;
    delay(10);
    _mp3.volume(v);
    delay(10);
    _mp3.loop(sound);
    delay(10);
  }

  void stop() {
    if(_powerOn) _mp3.stop();
  }

  void powerOn() {
    if(!_powerOn) {
      _powerOn = 1;
      _serial->begin(9600);
      digitalWrite(_powerPin, HIGH);
      delay(2000);
      init();
    }
  }

  void powerOff() {
    if(_powerOn) {
      _powerOn = 0;
      delay(100);
      _serial->flush();
      _serial->end();
      digitalWrite(_powerPin, LOW);
      pinMode(_txPin, INPUT);     
      pinMode(_rxPin, INPUT);    
     }
  }

protected:
  DFRobotDFPlayerMini _mp3;
  HardwareSerial *_serial;
  uint32_t _powerPin;
  uint32_t _rxPin;
  uint32_t _txPin;
  bool _powerOn;
};
#endif