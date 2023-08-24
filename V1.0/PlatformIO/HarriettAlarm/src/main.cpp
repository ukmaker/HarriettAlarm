/**
 * The Harriett Alarm Clock
 * Optionally, also runs Forth
*/
//#define HARRIETT_GOES_FORTH

#include "Buttons.h"
#include "View.h"
#include "epaper.h"
#include "lesson.h"
#include "sleep_in.h"
#include "work.h"

#include "Alarm.h"
#include "WavetableAlarm.h"

#include "STM32LowPower.h"
#include "TimerInterrupt_Generic.h"

#include <Arduino.h>
#include <STM32RTC.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#ifdef HARRIETT_GOES_FORTH
    #include "ForthConfiguration.h"
    #include "ForthVM.h"
    #include "STM32Dev.h"
    #include "UnsafeMemory.h"
    #include "local_syscalls.h"
    #include "syscalls.h"
#endif

#define TICK_MS 10

STM32Timer debounceTimer(TIM2);
HardwareTimer pwm;

int ticks = 0;

void startAlarm();
void stopAlarm();
void snoozeAlarm();

STM32RTC& rtc = STM32RTC::getInstance();

uint8_t setMode, lastSetMode;
uint8_t alarmState;
bool settingVolume = 0;

bool mp3Enabled = 0;

time_t now;

Alarm workAlarm(7, 0, 1, 10, work_bitmap, work_width, work_height);
Alarm lessonAlarm(12, 2, 2, 10, lesson_bitmap, lesson_width, lesson_height);
Alarm* activeAlarm = NULL;
Bitmap sleepInBitmap(sleep_in_bitmap, sleep_in_width, sleep_in_height);
ClockTime clockTime(12, 0);

View view(display);

WavetableAlarm wavAlarm;

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

/***************************************************
 * Interrupt handlers
 ***************************************************/
void debounceInterruptHandler()
{
    static unsigned int milliseconds = 0;
    milliseconds += TICK_MS;
    buttons.update(milliseconds);
}

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

void pwmInterruptHandler(void)
{
    TIM1->CCR1 = wavAlarm.nextSample();
}

/***************************************************
 * Manage the PWM for generating sounds
****************************************************/
void startPWM() {
   pwm.resume();
}

void stopPWM() {
    pwm.pause();
}

void initPWM() {
    pwm.setup(TIM1);
    pwm.setPWM(1, PA8, 48000, 50, pwmInterruptHandler);
}

void stopSound() {
    pwm.pause();
}

void startSound() {
    startPWM();
}

void initSound() {
    initPWM();
}

void playSound() {
    wavAlarm.start(activeAlarm->tone(), activeAlarm->volume());
    startSound();
}

/***************************************************
 * Set time and alarm
 ***************************************************/
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

void setModeIdle()
{
    setMode = SET_MODE_IDLE;
    view.updateClock(clockTime);
    view.clearSetTime();
    view.clearSetAlarmTime();
    view.clearSetAlarmTone();
    stopSound();
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

void bumpAlarmVolume(bool up)
{
    startSound();
    if (up) {
        activeAlarm->louder();
    } else {
        activeAlarm->quieter();
    }
    //mp3Alarm.play(activeAlarm->tone(), activeAlarm->volume());
    wavAlarm.start(activeAlarm->tone(), activeAlarm->volume());
    view.displayAlarmTime(activeAlarm);
}

void bumpTimeOrTone(bool hours, bool byFive) {
    // If we're setting the time then bump the hours
    switch (setMode) {
    case SET_MODE_TIME:
        clockTime.getRTC();
        if(hours) {
            clockTime.incrementHours(byFive);
        } else {
            clockTime.incrementMinutes(byFive);
        }
        clockTime.setRTC();
        view.updateClock(clockTime);

        break;

    case SET_MODE_ALARM_TIME:
        if(hours) {
            activeAlarm->time().incrementHours(byFive);
        } else {
            activeAlarm->time().incrementMinutes(byFive);
        }
        updateAlarmTime(activeAlarm);
        view.displayAlarmTime(activeAlarm);
        break;

    case SET_MODE_ALARM_TONE:

        if (activeAlarm != NULL) {
            if(hours) {
                activeAlarm->nextTone();
            } else {
                activeAlarm->prevTone();
            }
            view.displayAlarmTime(activeAlarm);
            playSound();
        }

        break;

    default:
        break;
    }
}

/***************************************************
 * Click handlers b2 b1 b3 b4
 ***************************************************/
void b1ClickHandler()
{
    if (alarmState == ALARM_STATE_SOUNDING) {
        snoozeAlarm();
    } else if (setMode == SET_MODE_ALARM_TONE) {
        bumpAlarmVolume(false);
    }
}

void b2ClickHandler()
{
    if (alarmState == ALARM_STATE_SOUNDING) {
        snoozeAlarm();
    } else if (setMode == SET_MODE_ALARM_TONE) {
        bumpAlarmVolume(true);
    }
}

void b1LongClickHandler(void)
{
    if (setMode != SET_MODE_ALARM_TONE && setMode != SET_MODE_TIME) {
        if (activeAlarm != NULL) {
            activateSleepIn();
        } else {
            activateWorkAlarm();
        }
    }
}

void b2LongClickHandler(void)
{
    if (setMode != SET_MODE_ALARM_TONE && setMode != SET_MODE_TIME) {
        if (activeAlarm != NULL) {
            activateSleepIn();
        } else {
            activateLessonAlarm();
        }
    }
}
/**
 * Long presses on both right buttons cycle through the settings
 * - If no alarm is active just activate or deactivate setting the time
 * - If an alarm is active cycle between setting the alarm time, alarm sound or nothing
 **/
void b1b2LongClickHandler(void)
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
        stopSound();
        break;

    case SET_MODE_IDLE:
    default:
        setMode = SET_MODE_ALARM_TIME;
        view.showSetAlarmTime();
        break;
    }
}

void b3ClickHandler(void)
{
    if (alarmState == ALARM_STATE_SOUNDING) {
        snoozeAlarm();
    } else {
        bumpTimeOrTone(true, false);
    }
}

void b4ClickHandler(void)
{
    if (alarmState == ALARM_STATE_SOUNDING || alarmState == ALARM_STATE_SNOOZING) {
        stopAlarm();
    } else {
        bumpTimeOrTone(false, false);
    }
}

void b3b4LongClickHandler()
{
    if (setMode == SET_MODE_TIME) {
        setModeIdle();
    } else {
        setModeSetTime();
    }
}

void b3RepeatClickHandler(void)
{
    bumpTimeOrTone(true, true);
}

void b4RepeatClickHandler(void)
{
    bumpTimeOrTone(false, true);
}

/***************************************
 * Timer triggered actions
****************************************/
void soundAlarm()
{
    alarmState = ALARM_STATE_SOUNDING;
    playSound();
    view.clearAlarmSnoozing();
}

void stopAlarm()
{
    alarmState = ALARM_STATE_IDLE;
    rtc.disableAlarm(STM32RTC::Alarm::ALARM_B);
    stopSound();
    view.clearAlarmSnoozing();
}

void snoozeAlarm()
{
    time_t epoc = rtc.getEpoch();
    alarmState = ALARM_STATE_SNOOZING;
    snoozeEpoch = epoc + 60 * ALARM_SNOOZE_MINUTES;
    rtc.setAlarmEpoch(snoozeEpoch, STM32RTC::Alarm_Match::MATCH_HHMMSS, STM32RTC::Alarm::ALARM_B);
    stopSound();
    view.showAlarmSnoozing(snoozeEpoch, epoc, 60 * ALARM_SNOOZE_MINUTES);
}

void sleepClock()
{
    //Serial.println("Sleeping");
    stopSound();
    digitalWrite(LED_BUILTIN, HIGH);
    display.hibernate();
    //Serial.end();
    pinMode(HOST_RX, INPUT_PULLUP);
    LowPower.attachInterruptWakeup(HOST_RX, serialInput, FALLING);
    HAL_SuspendTick();
    LowPower.deepSleep(60000);
    HAL_ResumeTick();
    //Serial.begin(115200);
    //Serial.println("Resume");
    digitalWrite(LED_BUILTIN, LOW);
}

/************************************************************************
 * Forth for fun and fancy
 ************************************************************************/
#ifdef HARRIETT_GOES_FORTH

uint8_t ram[8192];

UnsafeMemory mem(ram, 8192, 0x2000, rom, 8192, 0);

Syscall syscalls[40];

ForthVM vm(&mem, syscalls, 40);

void syscall_debug(ForthVM* vm)
{
    uint16_t action = vm->pop();
    uint8_t v;
    activeAlarm = &workAlarm;
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
        startSound();
        break;
    case 5:
        stopSound();
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
    case 8:
        activeAlarm->louder();
        playSound();
        break;
    case 9:
        activeAlarm->quieter();
        playSound();
        break;
    case 10:
        v = vm->pop();
        v += 2;
        activeAlarm->setVolume(v);
        playSound();
    case 11:
        v = activeAlarm->volume();
        vm->push(v);
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
  : +V 8 BREAKPOINT ; \n \
  : -V 9 BREAKPOINT ; \n \
  : V! 10 BREAKPOINT ; \n \
  : V@ 11 BREAKPOINT ; \n \
  \
";

#endif 

/***************************************
 * Initialisation
****************************************/
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

void setupButtonInterrupts()
{
    LowPower.attachInterruptWakeup(LESSON_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
    LowPower.attachInterruptWakeup(WORK_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
    LowPower.attachInterruptWakeup(SNOOZE_HH_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
    LowPower.attachInterruptWakeup(STOP_MM_BUTTON_PIN, buttonCallback, FALLING, SLEEP_MODE);
}

void attachEventHandlers()
{
    buttons.setB1ClickHandler(b1ClickHandler);
    buttons.setB2ClickHandler(b2ClickHandler);
    buttons.setB1LongClickHandler(b1LongClickHandler);
    buttons.setB2LongClickHandler(b2LongClickHandler);
    buttons.setB1B2LongClickHandler(b1b2LongClickHandler);

    buttons.setB3ClickHandler(b3ClickHandler);
    buttons.setB4ClickHandler(b4ClickHandler);
    buttons.setB3B4LongClickHandler(b3b4LongClickHandler);
    buttons.setB3RepeatClickHandler(b3RepeatClickHandler);
    buttons.setB4RepeatClickHandler(b4RepeatClickHandler);
}

void setup()
{

    setupRTC();
    view.setup();
    view.updateClock(clockTime);
    view.clearAlarmSnoozing();
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
    #ifdef HARRIETT_GOES_FORTH
        attachSyscalls();
        Serial.begin(115200); // Forth communications with the host
        vm.reset();
    #endif
    debounceTimer.attachInterruptInterval(TICK_MS * 1000, debounceInterruptHandler);
    initSound();
}

void loop()
{
    //if (!buttonEvent)
    #ifdef HARRIETT_GOES_FORTH
        vm.step();
    #endif
    
    buttonEvent = false;

    uint32_t ms, secs;
    now = rtc.getEpoch(&ms);
    secs = now * 1000 + ms;

    buttons.dispatchEvents();
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
        active = false;
        serialIn = false;
        expiry = rtc.getEpoch() + 10;
    } else if (rtc.getEpoch() > expiry) {
        if (setMode != SET_MODE_IDLE) {
            setModeIdle();
        }
        buttons.reset();
        sleepClock();
    }
}
