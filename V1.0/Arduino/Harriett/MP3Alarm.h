#ifndef HARRIETT_MP3ALARM_H
#define HARRIETT_MP3ALARM_H

#include "DFRobotDFPlayerMini.h"

static const uint8_t _volumes[] = { 50, 50, 80, 50, 30, 50, 80, 50 };

class MP3Alarm {



public:

  MP3Alarm() : _mp3() {

  }

    bool init(Stream &stream) {
      if(!_mp3.begin(stream)) {
        return 0;
      }

      _mp3.setTimeOut(500);
      _mp3.outputDevice(DFPLAYER_DEVICE_SD);

      return 1;
    }

  void play(uint8_t sound, uint8_t volumePercent) {
    // Acceptable sound values are 1-8
    sound = (sound % 8) + 1;
    // Volume for the MP3 widget is 0 - 30
    // apply the multiplier so that volumePercent == 50 is the same audible volume for all sounds
    if(volumePercent > 100) {
      volumePercent = 100;
    }
    uint32_t v = ((volumePercent * 30) / 100) * _volumes[sound] / 100;
    _mp3.volume(v);
    _mp3.loop(sound);
  }

  void stop() {
    _mp3.stop();
  }

protected:
  DFRobotDFPlayerMini _mp3;
};
#endif