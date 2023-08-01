/*
  AdvancedTimedWakeup

  This sketch demonstrates the usage of Internal Interrupts to wakeup a chip in deep sleep mode.

  In this sketch:
  - RTC date and time are configured.
  - Alarm is set to wake up the processor each 'atime' and called a custom alarm callback
  which increment a value and reload alarm with 'atime' offset.

  This example code is in the public domain.
*/

#include "epaper.h"
#include "STM32LowPower.h"
#include "sleep_in.h"
#include "work.h"
#include "lesson.h"
#include "Buttons.h"
#include "View.h"

#include "Alarm.h"
#include "MP3Alarm.h"

/*
  AdvancedTimedWakeup

  This sketch demonstrates the usage of Internal Interrupts to wakeup a chip in deep sleep mode.

  In this sketch:
  - RTC date and time are configured.
  - Alarm is set to wake up the processor each 'atime' and called a custom alarm callback
  which increment a value and reload alarm with 'atime' offset.

  This example code is in the public domain.
*/

#include "STM32LowPower.h"
#include <STM32RTC.h>

STM32RTC& rtc = STM32RTC::getInstance();

uint8_t setMode, lastSetMode;
uint8_t alarmState;
bool settingVolume = 0;

bool mp3Enabled = 0;

Alarm workAlarm(7, 0, 1, 20, work_bitmap, work_width, work_height);
Alarm lessonAlarm(12, 1, 2, 20, lesson_bitmap, lesson_width, lesson_height);
Alarm* activeAlarm = NULL;
Bitmap sleepInBitmap(sleep_in_bitmap, sleep_in_width, sleep_in_height);
ClockTime clockTime(12, 0);

View view(display);


#define MAX_VOLUME 20
#define MP3_BUSY PB0
#define MP3_POWER PB10
#define MP3_RX PB1
#define MP3_TX PB2
#define MP3_POWERUP_DELAY 1500
#define MP3_ALARM_TONES 8

MP3Alarm mp3Alarm(MP3_POWER, MP3_RX, MP3_TX);

#define SET_MODE_IDLE 0
#define SET_MODE_TIME 1
#define SET_MODE_ALARM_TIME 2
#define SET_MODE_ALARM_TONE 3

#define BY_ONE 0
#define BY_FIVE 1

#define LESSON_BUTTON_PIN PB7
#define WORK_BUTTON_PIN PB6
#define SNOOZE_HH_BUTTON_PIN PB8
#define STOP_MM_BUTTON_PIN PB9

volatile bool alarmInterrupt = 1;

#define ALARM_STATE_IDLE 0
#define ALARM_STATE_STARTING 1
#define ALARM_STATE_SOUNDING 2
#define ALARM_STATE_SNOOZING 3

#define ALARM_SNOOZE_MINUTES 10
time_t snoozeEpoch;

Buttons buttons(  LESSON_BUTTON_PIN,
                  WORK_BUTTON_PIN,
                  SNOOZE_HH_BUTTON_PIN,
                  STOP_MM_BUTTON_PIN);

time_t expiry;
volatile bool active;

void activateSleepIn() {
  view.setBitmap(sleepInBitmap);
  view.clearAlarmTime();
  view.clearSetAlarmTime();
  rtc.disableAlarm();
  activeAlarm = NULL;
  setMode = SET_MODE_IDLE;
}

void updateAlarmTime(Alarm* alarm) {
  rtc.setAlarmDay(1);
  rtc.setAlarmHours(alarm->time().h());
  rtc.setAlarmMinutes(alarm->time().m());
  rtc.setAlarmSeconds(5);
  rtc.enableAlarm(STM32RTC::MATCH_HHMMSS);
  activeAlarm = alarm;
}

void setActiveAlarm(Alarm* alarm) {
  view.setBitmap(alarm->bitmap());
  view.clearSetAlarmTime();
  view.displayAlarmTime(alarm);
  view.clearAlarmSnoozing();
  updateAlarmTime(alarm);
  setMode = SET_MODE_IDLE;
}

void activateWorkAlarm() {
  setActiveAlarm(&workAlarm);
}

void activateLessonAlarm() {
  setActiveAlarm(&lessonAlarm);
}

void setupRTC() {
  rtc.setClockSource(STM32RTC::LSE_CLOCK);
  rtc.setPrediv(127, 255);
  rtc.begin();
  rtc.setTime(12, 0, 0);
  rtc.setDate(1, 1, 1, 2023);
  clockTime.getRTC();

  pinMode(LED_BUILTIN, OUTPUT);

  // Configure low power
  LowPower.begin();
  //LowPower.enableWakeupFrom(&rtc, secondsInt, &atime);
}

/***************************
* Interrupt handlers
***************************/
void alarmMatch(void* data) {
  alarmInterrupt = 1;
  active = 1;
}

void buttonCallback() {
  //alarmInterrupt = 1;
  active = 1;
}

void secondsInt(void* data) {
  // do nothing, just be awake now
  active = 1;
}

void setModeIdle() {
  setMode = SET_MODE_IDLE;
  view.updateClock(clockTime);
  view.clearSetTime();
  view.clearSetAlarmTime();
  view.clearSetAlarmTone();
  mp3Alarm.stop();
}

void setModeSetTime() {
  setMode = SET_MODE_TIME;
  view.updateClock(clockTime);
  view.showSetTime();
}

void setModeSetWorkAlarmTime() {
  activateWorkAlarm();
  setMode = SET_MODE_ALARM_TIME;
  view.showSetAlarmTime();
}

void setModeSetLessonAlarmTime() {
  activateLessonAlarm();
  setMode = SET_MODE_ALARM_TIME;
  view.showSetAlarmTime();
}

void setModeSetWorkAlarmTone() {
  activateWorkAlarm();
  setMode = SET_MODE_ALARM_TONE;
  view.showSetAlarmTone();
}

void setModeSetLessonAlarmTone() {
  activateLessonAlarm();
  setMode = SET_MODE_ALARM_TONE;
  view.showSetAlarmTone();
}

void backRightLongClickHandler(void) {
  if (setMode != SET_MODE_ALARM_TONE) {
   // rtc.detachSecondsInterrupt();
    if (activeAlarm != NULL) {
      activateSleepIn();
    } else {
      activateLessonAlarm();
    }
    //rtc.attachSecondsInterrupt(secondsInt);
  }
}


void backLeftLongClickHandler(void) {
  if (setMode != SET_MODE_ALARM_TONE) {
    //rtc.detachSecondsInterrupt();
    if (activeAlarm != NULL) {
      activateSleepIn();
    } else {
      activateWorkAlarm();
    }
    //rtc.attachSecondsInterrupt(secondsInt);
  }
}

void backRightShortClickHandler() {
  if (setMode == SET_MODE_ALARM_TONE) {  // otherwise ignore
    if (activeAlarm != NULL) {
      activeAlarm->nextTone();
      view.displayAlarmTime(activeAlarm);
      mp3Alarm.stop();
      mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
    }
  }
}

void backLeftShortClickHandler() {
  if (setMode == SET_MODE_ALARM_TONE) {
    if (activeAlarm != NULL) {
      activeAlarm->prevTone();
      view.displayAlarmTime(activeAlarm);
      mp3Alarm.stop();
      mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
    }
  }
}


/**
* Long presses on both back buttons cycle through the settings
* - If no alarm is active just activate or deactivate setting the time
* - If an alarm is active cycle between setting the alarm time, alarm sound or nothing
**/
void setModeClickHandler(void) {
  if (activeAlarm == NULL) {
    if (setMode == SET_MODE_IDLE) {
      setModeSetTime();
    } else {
      setModeIdle();
    }
  } else {
    switch (setMode) {

      case SET_MODE_ALARM_TIME:
        setMode = SET_MODE_ALARM_TONE;
        view.clearSetAlarmTime();
        view.showSetAlarmTone();
        break;

      case SET_MODE_ALARM_TONE:
        setMode = SET_MODE_IDLE;
        view.clearSetAlarmTone();
        mp3Alarm.stop();
        break;

      case SET_MODE_IDLE:
      default:
        setMode = SET_MODE_ALARM_TIME;
        view.showSetAlarmTime();
        break;
    }
  }
}

void bumpHours(bool byFive) {
  // If we're setting the time then bump the hours
  //rtc.detachSecondsInterrupt();
  switch (setMode) {
    case SET_MODE_TIME:
      clockTime.getRTC();
      clockTime.incrementHours(byFive);
      clockTime.setRTC();
      setModeSetTime();
      break;

    case SET_MODE_ALARM_TIME:
      activeAlarm->time().incrementHours(byFive);
      updateAlarmTime(activeAlarm);
      view.displayAlarmTime(activeAlarm);
      break;

    case SET_MODE_ALARM_TONE:
      mp3Alarm.powerOn();
      mp3Alarm.stop();
      activeAlarm->louder();
      view.displayAlarmTime(activeAlarm);
      mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
      break;

    default:
      break;
  }
  //rtc.attachSecondsInterrupt(secondsInt);
}

void bumpMinutes(bool byFive) {
  // If we're setting the time then bump the hours
  //rtc.detachSecondsInterrupt();
  switch (setMode) {
    case SET_MODE_TIME:
      clockTime.getRTC();
      clockTime.incrementMinutes(byFive);
      clockTime.setRTC();
      setModeSetTime();
      break;

    case SET_MODE_ALARM_TIME:
      activeAlarm->time().incrementMinutes(byFive);
      updateAlarmTime(activeAlarm);
      view.displayAlarmTime(activeAlarm);
      break;

    case SET_MODE_ALARM_TONE:
      mp3Alarm.powerOn();
      mp3Alarm.stop();
      activeAlarm->quieter();
      mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
      view.displayAlarmTime(activeAlarm);
      break;

    default:
      break;
  }
  //rtc.attachSecondsInterrupt(secondsInt);
}

void snoozeClickHandler(void) {

  time_t epoc = rtc.getEpoch();
  // if an alarm is active, snooze it
  if (alarmState == ALARM_STATE_SOUNDING) {
    alarmState = ALARM_STATE_SNOOZING;
    snoozeEpoch = epoc + 60 * ALARM_SNOOZE_MINUTES;
    rtc.setAlarmEpoch(snoozeEpoch);
    mp3Alarm.stop();
    mp3Alarm.powerOff(); 
    view.showAlarmSnoozing(snoozeEpoch, epoc, 60 * ALARM_SNOOZE_MINUTES);
  } else {
    bumpHours(BY_ONE);
  }
}

void snoozeRepeatClickHandler(void) {
  bumpHours(BY_FIVE);
}

void stopClickHandler(void) {

  time_t epoc = rtc.getEpoch();
  // if an alarm is active, snooze it
  if (alarmState == ALARM_STATE_SOUNDING || alarmState == ALARM_STATE_SNOOZING) {
    alarmState = ALARM_STATE_IDLE;
    rtc.disableAlarm();
    mp3Alarm.stop();
    mp3Alarm.powerOff();
    view.clearAlarmSnoozing();
  } else {
    bumpMinutes(BY_ONE);
  }
}

void stopRepeatClickHandler(void) {
  bumpMinutes(BY_FIVE);
}

void soundAlarm() {
  mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
  view.clearAlarmSnoozing();
}

void setupButtonInterrupts() {
  LowPower.attachInterruptWakeup(LESSON_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
  LowPower.attachInterruptWakeup(WORK_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
  LowPower.attachInterruptWakeup(SNOOZE_HH_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
  LowPower.attachInterruptWakeup(STOP_MM_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
}

void attachEventHandlers() {
  buttons.setBackLeftLongClickHandler(backLeftLongClickHandler);
  buttons.setBackRightLongClickHandler(backRightLongClickHandler);
  buttons.setBackBothLongClickHandler(setModeClickHandler);
  buttons.setBackLeftClickHandler(backLeftShortClickHandler);
  buttons.setBackRightClickHandler(backRightShortClickHandler);

  buttons.setFrontRightClickHandler(snoozeClickHandler);
  buttons.setFrontLeftClickHandler(stopClickHandler);
  buttons.setFrontleftRepeatClickHandler(snoozeRepeatClickHandler);
  buttons.setFrontRightRepeatClickHandler(stopRepeatClickHandler);
}

void setup() {

  setupRTC();
  view.setup();
  view.updateClock(clockTime);
  view.clearAlarmSnoozing();

  setMode = SET_MODE_IDLE;
  alarmState = ALARM_STATE_IDLE;
  activeAlarm = NULL;
  rtc.attachInterrupt(alarmMatch);
  setupButtonInterrupts();
  attachEventHandlers();
  //rtc.attachSecondsInterrupt(secondsInt);
  LowPower.enableWakeupFrom(&rtc, alarmMatch);

  active = 1;
  expiry = 0;

  activateLessonAlarm();
soundAlarm();
}

void loop() { }

void floop() {

 //display.init(115200);
  
  time_t now = rtc.getEpoch();

  buttons.update();
  active = buttons.active();
  // how did we get here? Respond to user inputs first
  digitalWrite(LED_BUILTIN, !active);

  // Now update the time - does a complete refresh every 60 seconds
  clockTime.getRTC();
  if (clockTime.s() == 0) {
    //rtc.detachSecondsInterrupt();
    view.updateClock(clockTime);

    if (alarmState == ALARM_STATE_SNOOZING) {
      view.showAlarmSnoozing(snoozeEpoch, now, 60 * ALARM_SNOOZE_MINUTES);
    }
    //rtc.attachSecondsInterrupt(secondsInt);
  }

  // did an alarm go off?
  if (alarmInterrupt) {
    alarmInterrupt = 0;
    alarmState = ALARM_STATE_SOUNDING;
    soundAlarm();
  }
  
  if (alarmState == ALARM_STATE_SOUNDING) {
    active = 1;
  }

  if (active) {
    // wait five seconds before poweroff so we can catch any additional button clicks
    active = 0;
    expiry = rtc.getEpoch() + 5;
  } else if (rtc.getEpoch() > expiry) {
    if (setMode != SET_MODE_IDLE) {
      setModeIdle();
    }
    mp3Alarm.powerOffNow();
    digitalWrite(PB10,LOW);
    display.hibernate();
    //HAL_SuspendTick();
    LowPower.deepSleep(1000);
    //HAL_ResumeTick();
  }
}
