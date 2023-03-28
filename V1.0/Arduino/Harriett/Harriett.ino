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
uint8_t alarmMode, alarmState;
bool settingVolume = 0;

bool mp3Enabled = 0;

Alarm workAlarm(7, 0, 1, 20, work_bitmap, work_width, work_height);
Alarm lessonAlarm(12, 1, 2, 20, lesson_bitmap, lesson_width, lesson_height);
Bitmap sleepInBitmap(sleep_in_bitmap, sleep_in_width, sleep_in_height);
ClockTime clockTime(12, 0);

View view(display);

MP3Alarm mp3Alarm;

#define MAX_VOLUME 20
#define MP3_BUSY PB12
#define MP3_POWER PB11
#define MP3_POWERUP_DELAY 1500
#define MP3_ALARM_TONES 8

#define SET_MODE_IDLE 0
#define SET_MODE_TIME 1
#define SET_MODE_LESSON 2
#define SET_MODE_WORK 3

#define BY_ONE 0
#define BY_FIVE 1

#define ALARM_MODE_SLEEP_IN 0
#define ALARM_MODE_LESSON 1
#define ALARM_MODE_WORK 2

#define LESSON_BUTTON_PIN 25
#define LESSON_LED_PIN 24

#define WORK_BUTTON_PIN 23
#define WORK_LED_PIN 22

#define SNOOZE_HH_BUTTON_PIN 21
#define SNOOZE_HH_LED_PIN 20

#define STOP_MM_BUTTON_PIN 19
#define STOP_MM_LED_PIN 18

#define LED_R_PIN PB12
#define LED_G_PIN PB13
#define LED_B_PIN PB14

volatile bool alarmInterrupt = 0;

#define ALARM_STATE_IDLE 0
#define ALARM_STATE_STARTING 1
#define ALARM_STATE_SOUNDING 2
#define ALARM_STATE_SNOOZING 3

#define ALARM_SNOOZE_MINUTES 10
time_t snoozeEpoch;

TwoByTwo twobytwo(LED_R_PIN, LED_G_PIN, LED_B_PIN,
                  LESSON_BUTTON_PIN, LESSON_LED_PIN,
                  WORK_BUTTON_PIN, WORK_LED_PIN,
                  SNOOZE_HH_BUTTON_PIN, SNOOZE_HH_LED_PIN,
                  STOP_MM_BUTTON_PIN, STOP_MM_LED_PIN);

Buttons buttons(twobytwo);

time_t expiry;
volatile bool active;

void setSleepIn() {
  view.setBitmap(sleepInBitmap);
  view.clearAlarmTime();
  view.clearSetAlarmTime();
  rtc.disableAlarm();
  alarmMode = ALARM_MODE_SLEEP_IN;
  setMode = SET_MODE_IDLE;
}


void setAlarm(Alarm& alarm, uint8_t mode) {
  view.setBitmap(alarm.bitmap());
  view.clearSetAlarmTime();
  view.displayAlarmTime(alarm);
  view.clearAlarmSnoozing();
  rtc.setAlarmHours(alarm.time().h());
  rtc.setAlarmMinutes(alarm.time().m());
  rtc.setAlarmSeconds(5);
  rtc.enableAlarm(STM32RTC::MATCH_HHMMSS);
  alarmMode = mode;
  setMode = SET_MODE_IDLE;
}

void setWork() {
  setAlarm(workAlarm, ALARM_MODE_WORK);
}

void setLesson() {
  setAlarm(lessonAlarm, ALARM_MODE_LESSON);
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
  active = 1;
}

void secondsInt(void* data) {
  // do nothing, just be awake now
  active = 1;
}


void lessonLongClickHandler(void) {
  rtc.detachSecondsInterrupt();
  if (alarmMode == ALARM_MODE_LESSON) {
    alarmMode = ALARM_MODE_SLEEP_IN;
    setSleepIn();
  } else {
    alarmMode = ALARM_MODE_LESSON;
    setLesson();
  }
  rtc.attachSecondsInterrupt(secondsInt);
}


void workLongClickHandler(void) {
  rtc.detachSecondsInterrupt();
  if (alarmMode == ALARM_MODE_WORK) {
    alarmMode = ALARM_MODE_SLEEP_IN;
    setSleepIn();
  } else {
    alarmMode = ALARM_MODE_WORK;
    setWork();
  }
  rtc.attachSecondsInterrupt(secondsInt);
}

void setTimeClickHandler(void) {
  switch (alarmMode) {
    case ALARM_MODE_WORK:
      setWorkAlarmTime();
      view.showSetAlarm();
      break;

    case ALARM_MODE_LESSON:
      setLessonAlarmTime();
      view.showSetAlarm();
      break;

    default:
      setTime();
      view.showSetTime();
      break;
  }
}

void setWorkAlarmTime() {
  setMode = SET_MODE_WORK;
  view.displayAlarmTime(workAlarm);
  rtc.setAlarmDay(1);
  rtc.setAlarmHours(workAlarm.time().h());
  rtc.setAlarmMinutes(workAlarm.time().m());
  rtc.setAlarmSeconds(5);
  rtc.enableAlarm(STM32RTC::MATCH_HHMMSS);
}

void setLessonAlarmTime() {
  setMode = SET_MODE_LESSON;
  view.displayAlarmTime(lessonAlarm);
  rtc.setAlarmDay(1);
  rtc.setAlarmHours(lessonAlarm.time().h());
  rtc.setAlarmMinutes(lessonAlarm.time().m());
  rtc.setAlarmSeconds(5);
  rtc.enableAlarm(STM32RTC::MATCH_HHMMSS);
}

void setTime() {
  setMode = SET_MODE_TIME;
  view.updateClock(clockTime);
}

void bumpHours(bool byFive) {
  // If we're setting the time then bump the hours
  rtc.detachSecondsInterrupt();
  switch (setMode) {
    case SET_MODE_TIME:
      clockTime.getRTC();
      clockTime.incrementHours(byFive);
      clockTime.setRTC();
      setTime();
      break;

    case SET_MODE_LESSON:
      lessonAlarm.time().incrementHours(byFive);
      setLessonAlarmTime();
      break;

    case SET_MODE_WORK:
      workAlarm.time().incrementHours(byFive);
      setWorkAlarmTime();
      break;

    default:
      break;
  }
  rtc.attachSecondsInterrupt(secondsInt);
}

void bumpMinutes(bool byFive) {
  // If we're setting the time then bump the hours
  rtc.detachSecondsInterrupt();
  switch (setMode) {
    case SET_MODE_TIME:
      clockTime.getRTC();
      clockTime.incrementMinutes(byFive);
      clockTime.setRTC();
      setTime();
      break;

    case SET_MODE_LESSON:
      lessonAlarm.time().incrementMinutes(byFive);
      setLessonAlarmTime();
      break;

    case SET_MODE_WORK:
      workAlarm.time().incrementMinutes(byFive);
      setWorkAlarmTime();
      break;

    default:
      break;
  }
  rtc.attachSecondsInterrupt(secondsInt);
}

void snoozeClickHandler(void) {

  time_t epoc = rtc.getEpoch();
  // if an alarm is active, snooze it
  if (alarmState == ALARM_STATE_SOUNDING) {
    alarmState = ALARM_STATE_SNOOZING;
    snoozeEpoch = epoc + 60 * ALARM_SNOOZE_MINUTES;
    rtc.setAlarmEpoch(snoozeEpoch);
    mp3Alarm.stop();
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
  if (alarmState == ALARM_STATE_SOUNDING) {
    alarmState = ALARM_STATE_IDLE;
    rtc.disableAlarm();
    mp3Alarm.stop();
    view.clearAlarmSnoozing();
  } else {
    bumpMinutes(BY_ONE);
  }
}

void stopRepeatClickHandler(void) {
  bumpMinutes(BY_FIVE);
}

void soundAlarm() {
  mp3Alarm.play(1, 50);
  view.clearAlarmSnoozing();
}

void setupButtonInterrupts() {
  LowPower.attachInterruptWakeup(LESSON_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
  LowPower.attachInterruptWakeup(WORK_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
  LowPower.attachInterruptWakeup(SNOOZE_HH_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
  LowPower.attachInterruptWakeup(STOP_MM_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
}

void attachEventHandlers() {
  buttons.setWorkLongClickHandler(workLongClickHandler);
  buttons.setLessonLongClickHandler(lessonLongClickHandler);
  buttons.setSetTimeClickHandler(setTimeClickHandler);

  buttons.setSnoozeClickHandler(snoozeClickHandler);
  buttons.setStopClickHandler(stopClickHandler);
  buttons.setSnoozeRepeatClickHandler(snoozeRepeatClickHandler);
  buttons.setStopRepeatClickHandler(stopRepeatClickHandler);
}

void setup() {

  setupRTC();
  view.init();
  view.updateClock(clockTime);
  view.clearAlarmSnoozing();
  Serial.begin(9600);
  mp3Alarm.init(Serial);
  //mp3Alarm.play(1,50);

  setMode = SET_MODE_IDLE;
  alarmState = ALARM_STATE_IDLE;
  alarmMode = ALARM_MODE_SLEEP_IN;
  setLesson();
  setupButtonInterrupts();
  attachEventHandlers();
  rtc.attachSecondsInterrupt(secondsInt);
  LowPower.enableWakeupFrom(&rtc, alarmMatch);
  active = 1;
  expiry = 0;
}

void loop() {

  time_t now = rtc.getEpoch();

  buttons.update();
  active = buttons.active();
  // how did we get here? Respond to user inputs first
  digitalWrite(LED_BUILTIN, !active);
  // Now update the time - does a complete refresh every 60 seconds
  clockTime.getRTC();
  if (clockTime.s() == 0) {
    rtc.detachSecondsInterrupt();
    view.updateClock(clockTime);
    rtc.attachSecondsInterrupt(secondsInt);
    if(alarmState == ALARM_STATE_SNOOZING) {
      view.showAlarmSnoozing(snoozeEpoch, now, 60 * ALARM_SNOOZE_MINUTES);
    }
  }

  // did an alarm go off?
  if (alarmInterrupt) {
    alarmInterrupt = 0;
    alarmState = ALARM_STATE_STARTING;
  }

  // is an alarm sounding?
  if (alarmState == ALARM_STATE_STARTING) {
    alarmState = ALARM_STATE_SOUNDING;
    soundAlarm();
    active = 1;
  }

  if (active) {
    active = 0;
    expiry = rtc.getEpoch() + 5;
  } else if (rtc.getEpoch() > expiry) {
    if (setMode != SET_MODE_IDLE) {
      setMode = SET_MODE_IDLE;
      view.clearSetTime();
      view.clearSetAlarmTime();
    }
    display.hibernate();
    LowPower.sleep();
  }
}
