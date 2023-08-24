#ifndef HARRIETT_WAVALARM_H
#define HARRIETT_WAVALARM_H

#include <Arduino.h>

#define WAV_ALARM_MAX_VOL 2047

class WavetableAlarm {

    public:

    WavetableAlarm() {}

    /**
     * tone selects the alarm tone (currently unused)
     * vol sets the volume: 1 to 10
    */
    void start(uint8_t tone, uint8_t vol) {
        volume = WAV_ALARM_MAX_VOL * vol / 10;
        for(uint32_t i=0; i<1024; i++) {
            wav[i] = volume / 2 + volume / 2 * alarmTone(tone, i);
        }
        samplePtr = 0;
        sampleInc = 65536;
    }

    /**
     * Normally called from a timer interrupt handler
    */
    uint16_t nextSample() {
	  samplePtr += sampleInc;
	  return warble(samplePtr) * wav[(samplePtr >> 16) & 1023];
    }

    protected:



    float sine(uint32_t i) {
        float t = (float)i / 512.0;
        return sinf(pi * 2 * t);
    }

    float triangle(uint32_t i) {
        if(i < 512) {
            float v = (float)i / 256;
            return v - 1;
        }

        float v = (float)(i-512) / 256;
        return 1 - v;
    }

    float square(uint32_t i) {
        if(i < 512) {
            return -1;
        }

        return 1;
    }

    float sawtooth(uint32_t i) {
        float v = (float)i / 512;
        return v - 1;
    }

    float modulate(float v) {
        return (v + 1.0) / 2.0;
    }

    float alarmTone(uint8_t tone, uint32_t i) {
        switch(tone) {
            case 1:
                // sine modulated by a square wave
                return sine(i * 32) * modulate(square(i));
            case 2:
                // two-tone
                return (sine(i * 32) + sine(i * 35)) * modulate(square(i)) / 2;
            default:
                return sine(i * 32) * modulate(square(i));
        }
    }

    float warble(uint32_t i) {
        // dddd....dddd....
        i = i >> 27;
        if(i > 8) return 0;
        if(i % 2 == 0) return 0;
        return 1;
    }

    float pi = 3.14159265453;

    uint16_t wav[1024];
    uint32_t samplePtr = 0;
    uint32_t sampleInc = 0;  
    uint16_t volume;  
};

#endif