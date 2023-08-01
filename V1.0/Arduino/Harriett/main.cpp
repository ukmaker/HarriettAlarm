/*
  AdvancedTimedWakeup

  This sketch demonstrates the usage of Internal Interrupts to wakeup a chip in deep sleep mode.

  In this sketch:
  - RTC date and time are configured.
  - Alarm is set to wake up the processor each 'atime' and called a custom alarm callback
  which increment a value and reload alarm with 'atime' offset.

  This example code is in the public domain.
*/

#include "Buttons.h"
#include "STM32LowPower.h"
#include "View.h"
#include "epaper.h"
#include "lesson.h"
#include "sleep_in.h"
#include "work.h"

#include "Alarm.h"
#include "MP3Alarm.h"

#include "ForthConfiguration.h"
#include "ForthVM.h"
#include "UnsafeMemory.h"
#include "local_syscalls.h"
#include "syscalls.h"
#include <Arduino.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "STM32Dev.h"
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

void startAlarm();
void stopAlarm();
void snoozeAlarm();

STM32RTC& rtc = STM32RTC::getInstance();

uint8_t setMode, lastSetMode;
uint8_t alarmState;
bool settingVolume = 0;

bool mp3Enabled = 0;

time_t now;

Alarm workAlarm(7, 0, 1, 20, work_bitmap, work_width, work_height);
Alarm lessonAlarm(12, 1, 2, 20, lesson_bitmap, lesson_width, lesson_height);
Alarm* activeAlarm = NULL;
Bitmap sleepInBitmap(sleep_in_bitmap, sleep_in_width, sleep_in_height);
ClockTime clockTime(12, 0);

View view(display);

#define MAX_VOLUME 20
#define MP3_BUSY PB0
#define MP3_POWER PB10
#define MP3_RX PA3
#define MP3_TX PA2
#define MP3_POWERUP_DELAY 1500
#define MP3_ALARM_TONES 8

MP3Alarm mp3Alarm(MP3_POWER, &Serial2, MP3_RX, MP3_TX);

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

#define HOST_RX PA10

time_t snoozeEpoch;

Buttons buttons(LESSON_BUTTON_PIN,
    WORK_BUTTON_PIN,
    SNOOZE_HH_BUTTON_PIN,
    STOP_MM_BUTTON_PIN);

time_t expiry;
volatile bool active;
volatile bool buttonEvent;
volatile bool serialIn;
volatile bool updateTick;

void activateSleepIn()
{
    view.setBitmap(sleepInBitmap);
    view.clearAlarmTime();
    view.clearSetAlarmTime();
    rtc.disableAlarm(STM32RTC::Alarm::ALARM_B);
    activeAlarm = NULL;
    setMode = SET_MODE_IDLE;
}

void updateAlarmTime(Alarm* alarm)
{
    rtc.setAlarmDay(1, STM32RTC::Alarm::ALARM_B);
    rtc.setAlarmHours(alarm->time().h(), STM32RTC::Alarm::ALARM_B);
    rtc.setAlarmMinutes(alarm->time().m(), STM32RTC::Alarm::ALARM_B);
    rtc.setAlarmSeconds(5, STM32RTC::Alarm::ALARM_B);
    rtc.enableAlarm(STM32RTC::MATCH_HHMMSS, STM32RTC::Alarm::ALARM_B);
    activeAlarm = alarm;
}

void setActiveAlarm(Alarm* alarm)
{
    view.setBitmap(alarm->bitmap());
    view.clearSetAlarmTime();
    view.displayAlarmTime(alarm);
    view.clearAlarmSnoozing();
    updateAlarmTime(alarm);
    setMode = SET_MODE_IDLE;
}

void activateWorkAlarm()
{
    setActiveAlarm(&workAlarm);
}

void activateLessonAlarm()
{
    setActiveAlarm(&lessonAlarm);
}

void setupRTC()
{
    rtc.setClockSource(STM32RTC::LSE_CLOCK);
    rtc.setPrediv(127, 255);
    rtc.begin();
    rtc.setTime(12, 0, 0);
    rtc.setDate(1, 1, 1, 2023);
    clockTime.getRTC();

    pinMode(LED_BUILTIN, OUTPUT);

    // Configure low power
    LowPower.begin();
}

/***************************
 * Interrupt handlers
 ***************************/
void alarmMatch(void* data)
{
    alarmInterrupt = true;
}

void buttonCallback()
{
    buttonEvent = true;
}

void minuteTick(void* data)
{
    updateTick = true;
}

void serialInput()
{
    serialIn = true;
}

void setModeIdle()
{
    setMode = SET_MODE_IDLE;
    view.updateClock(clockTime);
    view.clearSetTime();
    view.clearSetAlarmTime();
    view.clearSetAlarmTone();
    mp3Alarm.stop();
}

void setModeSetTime()
{
    setMode = SET_MODE_TIME;
    view.updateClock(clockTime);
    view.clearSetAlarmTime();
    view.clearSetAlarmTone();
    view.showSetTime();
}

void setModeSetWorkAlarmTime()
{
    activateWorkAlarm();
    setMode = SET_MODE_ALARM_TIME;
    view.showSetAlarmTime();
}

void setModeSetLessonAlarmTime()
{
    activateLessonAlarm();
    setMode = SET_MODE_ALARM_TIME;
    view.showSetAlarmTime();
}

void setModeSetWorkAlarmTone()
{
    activateWorkAlarm();
    setMode = SET_MODE_ALARM_TONE;
    view.showSetAlarmTone();
}

void setModeSetLessonAlarmTone()
{
    activateLessonAlarm();
    setMode = SET_MODE_ALARM_TONE;
    view.showSetAlarmTone();
}

void backRightLongClickHandler(void)
{
    if (setMode != SET_MODE_ALARM_TONE) {
        if (activeAlarm != NULL) {
            activateSleepIn();
        } else {
            activateLessonAlarm();
        }
    }
}

void backLeftLongClickHandler(void)
{
    if (setMode != SET_MODE_ALARM_TONE) {
        if (activeAlarm != NULL) {
            activateSleepIn();
        } else {
            activateWorkAlarm();
        }
    }
}

void backRightShortClickHandler()
{
    Serial.println("Click 1");
    if (setMode == SET_MODE_ALARM_TONE) { // otherwise ignore
        if (activeAlarm != NULL) {
            activeAlarm->nextTone();
            view.displayAlarmTime(activeAlarm);
            mp3Alarm.stop();
            mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
        }
    }
}

void backLeftShortClickHandler()
{
    Serial.println("Click 2");
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
void setModeClickHandler(void)
{
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

void setTimeClickHandler() {
    if (setMode == SET_MODE_TIME) {
        setModeIdle();
    } else {
        setModeSetTime();
    }    
}

void bumpHours(bool byFive)
{
    // If we're setting the time then bump the hours
    switch (setMode) {
    case SET_MODE_TIME:
        clockTime.getRTC();
        clockTime.incrementHours(byFive);
        clockTime.setRTC();
        view.updateClock(clockTime);
    
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
}

void bumpMinutes(bool byFive)
{
    // If we're setting the time then bump the hours
    switch (setMode) {
    case SET_MODE_TIME:
        clockTime.getRTC();
        clockTime.incrementMinutes(byFive);
        clockTime.setRTC();
        view.updateClock(clockTime);
    
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
}

void snoozeClickHandler(void)
{
    if (alarmState == ALARM_STATE_SOUNDING) {
        snoozeAlarm();
    } else {
        bumpHours(BY_ONE);
    }
}

void snoozeRepeatClickHandler(void)
{
    bumpHours(BY_FIVE);
}

void stopClickHandler(void)
{
    if (alarmState == ALARM_STATE_SOUNDING || alarmState == ALARM_STATE_SNOOZING) {
        stopAlarm();
    } else {
        bumpMinutes(BY_ONE);
    }
}

void stopRepeatClickHandler(void)
{
    bumpMinutes(BY_FIVE);
}

void soundAlarm()
{
    alarmState = ALARM_STATE_SOUNDING;
    mp3Alarm.init();
    mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
    view.clearAlarmSnoozing();
}

void stopAlarm()
{
    alarmState = ALARM_STATE_IDLE;
    rtc.disableAlarm(STM32RTC::Alarm::ALARM_B);
    mp3Alarm.stop();
    mp3Alarm.powerOff();
    view.clearAlarmSnoozing();
}

void snoozeAlarm()
{
    time_t epoc = rtc.getEpoch();
    alarmState = ALARM_STATE_SNOOZING;
    snoozeEpoch = epoc + 60 * ALARM_SNOOZE_MINUTES;
    rtc.setAlarmEpoch(snoozeEpoch, STM32RTC::Alarm_Match::MATCH_HHMMSS, STM32RTC::Alarm::ALARM_B);
    mp3Alarm.stop();
    mp3Alarm.powerOff();
    view.showAlarmSnoozing(snoozeEpoch, epoc, 60 * ALARM_SNOOZE_MINUTES);
}

void sleepClock()
{
    mp3Alarm.powerOff();
    digitalWrite(LED_BUILTIN, HIGH);
    display.hibernate();
    Serial.end();
    pinMode(HOST_RX, INPUT_PULLUP);
    LowPower.attachInterruptWakeup(HOST_RX, serialInput, FALLING);
    HAL_SuspendTick();
    LowPower.deepSleep(60000);
    HAL_ResumeTick();
    Serial.begin(115200);
    digitalWrite(LED_BUILTIN, LOW);
}

void setupButtonInterrupts()
{
    LowPower.attachInterruptWakeup(LESSON_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
    LowPower.attachInterruptWakeup(WORK_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
    LowPower.attachInterruptWakeup(SNOOZE_HH_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
    LowPower.attachInterruptWakeup(STOP_MM_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
}

void attachEventHandlers()
{
    buttons.setBackLeftLongClickHandler(backLeftLongClickHandler);
    buttons.setBackRightLongClickHandler(backRightLongClickHandler);
    buttons.setBackBothLongClickHandler(setModeClickHandler);
    buttons.setBackLeftClickHandler(backLeftShortClickHandler);
    buttons.setBackRightClickHandler(backRightShortClickHandler);

   // buttons.setFrontBothLongClickHandler(setTimeClickHandler);
    buttons.setFrontRightClickHandler(snoozeClickHandler);
    buttons.setFrontLeftClickHandler(stopClickHandler);
    buttons.setFrontleftRepeatClickHandler(snoozeRepeatClickHandler);
    buttons.setFrontRightRepeatClickHandler(stopRepeatClickHandler);
}

/************************************************************************
 * Forth
 ************************************************************************/

uint8_t ram[8192];

UnsafeMemory mem(ram, 8192, 0x2000, rom, 8192, 0);

Syscall syscalls[40];

ForthVM vm(&mem, syscalls, 40);

void syscall_debug(ForthVM* vm)
{
    uint16_t action = vm->pop();
    switch (action) {
    case 1:
        soundAlarm();
        break;
    case 2:
        stopAlarm();
        break;
    case 3:
        snoozeAlarm();
        break;
    case 4:
        mp3Alarm.powerOn();
        break;
    case 5:
        mp3Alarm.powerOff();
        break;
    case 6: {
        vm->push(active);
        vm->push(alarmState);
        vm->push(now);
        vm->push(expiry);
        vm->push(snoozeEpoch);
        break;
    }
    case 7:
        sleepClock();
        break;
    default:
        break;
    }
}

void attachSyscalls()
{
    vm.addSyscall(SYSCALL_DEBUG, syscall_debug);
    vm.addSyscall(SYSCALL_TYPE, syscall_type);
    vm.addSyscall(SYSCALL_TYPELN, syscall_typeln);
    vm.addSyscall(SYSCALL_DOT, syscall_dot);
    vm.addSyscall(SYSCALL_GETC, syscall_getc);
    vm.addSyscall(SYSCALL_PUTC, syscall_putc);
    vm.addSyscall(SYSCALL_INLINE, syscall_inline);
    vm.addSyscall(SYSCALL_FLUSH, syscall_flush);
    vm.addSyscall(SYSCALL_NUMBER, syscall_number);
    vm.addSyscall(SYSCALL_H_AT, syscall_read_host);
    vm.addSyscall(SYSCALL_H_STORE, syscall_write_host);
    vm.addSyscall(SYSCALL_D_ADD, syscall_add_double);
    vm.addSyscall(SYSCALL_D_SUB, syscall_sub_double);
    vm.addSyscall(SYSCALL_D_DIV, syscall_mul_double);
    vm.addSyscall(SYSCALL_D_MUL, syscall_div_double);

    vm.addSyscall(SYSCALL_D_SR, syscall_sr_double);
    vm.addSyscall(SYSCALL_D_SL, syscall_sl_double);
    vm.addSyscall(SYSCALL_D_AND, syscall_and_double);
    vm.addSyscall(SYSCALL_D_OR, syscall_or_double);

    vm.addSyscall(SYSCALL_DOTC, syscall_dot_c);

    vm.addSyscall(SYSCALL_FCLOSE, syscall_fclose);
    vm.addSyscall(SYSCALL_FOPEN, syscall_fopen);
    vm.addSyscall(SYSCALL_FREAD, syscall_fread);
}

char* source_code = "\
  : START 1 BREAKPOINT ; \n \
  : STOP 2 BREAKPOINT ; \n \
  : SNOOZE 3 BREAKPOINT ; \n \
  : ON 4 BREAKPOINT ; \n \
  : OFF 5 BREAKPOINT ; \n \
  : Q 6 BREAKPOINT \n \
    .\" SNOOZE \" . CRET \n \
    .\" EXPIRY \" . CRET \n \
    .\" NOW    \" . CRET \n \
    .\" STATE  \" . CRET \n \
    .\" ACTIVE \" . CRET ; \n \
  : SLEEP 7 BREAKPOINT ; \n \
  \
";

void setup()
{

    setupRTC();
    view.setup();
    view.updateClock(clockTime);
    view.clearAlarmSnoozing();
    mp3Alarm.powerOff();
    setMode = SET_MODE_IDLE;
    alarmState = ALARM_STATE_IDLE;
    activeAlarm = NULL;
    alarmInterrupt = false;
    buttonEvent = false;
    active = true;
    expiry = 0;
    serialIn = true;

    /**
     * Set up interrupts.
     * Use AlarmA for the actual clock's alarm
     * Use the wakeup timer to wake the clock every 60 seconds to
     * update the time
     *
     * Button interrupt handler used for setting time, alarms etc
     *
     * During sleep mode, disable UART1 and set its Rx pin to
     * be an input. This is then used to wakeup the CPU if its
     * state changes, at which point UART1 is re-enabled
     */
    rtc.attachInterrupt(alarmMatch, NULL, STM32RTC::Alarm::ALARM_B);
    setupButtonInterrupts();
    attachEventHandlers();

    rtc.setAlarmDay(1);
    rtc.setAlarmHours(0);
    rtc.setAlarmMinutes(0);
    rtc.setAlarmSeconds(60);
    rtc.enableAlarm(STM32RTC::MATCH_SS);
    rtc.attachInterrupt(minuteTick);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, 1);
    attachSyscalls();
    Serial.begin(115200); // Forth communications with the host
    vm.reset();
}

void loop()
{   
    if(!buttonEvent) vm.step();
    buttonEvent = false;

    uint32_t ms, secs;
    now = rtc.getEpoch(&ms);
    secs = now * 1000 + ms;

    buttons.update(secs);
    active = buttons.active();

    // Now update the time - does a complete refresh every 60 seconds
    if (updateTick) {
      updateTick = false;
      clockTime.getRTC();
      view.updateClock(clockTime);

      if (alarmState == ALARM_STATE_SNOOZING) {
        // update the snooze progress bar
        view.showAlarmSnoozing(snoozeEpoch, now, 60 * ALARM_SNOOZE_MINUTES);
      }
    }

    // did an alarm go off?
    if (alarmInterrupt) {
        alarmInterrupt = 0;
        soundAlarm();
    }

    if (alarmState == ALARM_STATE_SOUNDING) {
        active = true;
    }

    if (active || serialIn) {
        // wait five seconds before poweroff so we can catch any additional button clicks
        active = false;
        expiry = rtc.getEpoch() + (serialIn ? 10 : 1);
        serialIn = false;
    } else if (rtc.getEpoch() > expiry) {
        if (setMode != SET_MODE_IDLE) {
            setModeIdle();
        }
        sleepClock();
    }
}
