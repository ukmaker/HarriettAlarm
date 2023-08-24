#ifndef HARRIETT_VIEW_H
#define HARRIETT_VIEW_H

#include "epaper.h"
#include "Bitmap.h"
#include "Alarm.h"

#define BITMAP_W 143
#define BITMAP_X (296 - BITMAP_W)
#define BITMAP_H 128
#define BITMAP_Y (128 - BITMAP_H)

#define TIME_WINDOW_X 0
#define TIME_WINDOW_Y 40
#define TIME_WINDOW_W 144
#define TIME_WINDOW_H 32
#define TIME_CURSOR_X (TIME_WINDOW_X + 0)
#define TIME_CURSOR_Y (TIME_WINDOW_Y + 30)

#define TIME_UNDERLINE_X 0
#define TIME_UNDERLINE_Y 80
#define TIME_UNDERLINE_W (296 / 2)
#define TIME_UNDERLINE_H 2

#define ALARM_WINDOW_X 8
#define ALARM_WINDOW_Y 96
#define ALARM_WINDOW_W 120
#define ALARM_WINDOW_H 16
#define ALARM_CURSOR_X (ALARM_WINDOW_X + 0)
#define ALARM_CURSOR_Y (ALARM_WINDOW_Y + 14)

#define ALARM_UNDERLINE_X 0
#define ALARM_UNDERLINE_Y 112
#define ALARM_UNDERLINE_W (296 / 4)
#define ALARM_UNDERLINE_H 2

#define ALARM_TONE_UNDERLINE_X (296 / 4)
#define ALARM_TONE_UNDERLINE_Y 112
#define ALARM_TONE_UNDERLINE_W (296 / 4)
#define ALARM_TONE_UNDERLINE_H 2

#define ALARM_SNOOZE_BAR_X 0
#define ALARM_SNOOZE_BAR_Y 120
#define ALARM_SNOOZE_BAR_W (296 / 3)
#define ALARM_SNOOZE_BAR_H 8
#define ALARM_SNOOZE_BAR_B 8

class PrintString : public Print, public String {
public:
  size_t write(uint8_t data) override {
    return concat(char(data));
  };
};

class View {

public:

  View(GxEPD2_BW<GxEPD2_290_T94_V2, 128> &display)
    : _display(display) {}
  ~View() {}

  void setup() {
    init();
    cls();
  }

  void init() {
    delay(100);
    _display.init(0);
    delay(1000);
    _display.setRotation(1);
    setFont(&FreeMonoBold24pt7b);
    _display.setTextColor(GxEPD_BLACK);
  }

  void cls() {
    _display.setFullWindow();
    _display.firstPage();
    do {
      _display.fillScreen(GxEPD_WHITE);
    } while (_display.nextPage());
  }

  void setFont(const GFXfont *f) {
    if (_font != f) {
      _font = f;
      _display.setFont(f);
    }
  }

  void updateClock(ClockTime &time) {

    PrintString valueString;

    valueString.print(time.h(), DEC);
    valueString.print(':');
    if (time.m() < 10) valueString.print("0");
    valueString.print(time.m(), DEC);

    setFont(&FreeMonoBold24pt7b);
    _display.setPartialWindow(TIME_WINDOW_X, TIME_WINDOW_Y, TIME_WINDOW_W, TIME_WINDOW_H);
    _display.firstPage();
    do {
      _display.setCursor(TIME_CURSOR_X, TIME_CURSOR_Y);
      _display.print(valueString);
    } while (_display.nextPage());
  }


  void clearAlarmTime() {
    _display.setPartialWindow(ALARM_WINDOW_X, ALARM_WINDOW_Y, ALARM_WINDOW_W, ALARM_WINDOW_H);
    _display.firstPage();
    do {
      _display.fillRect(ALARM_WINDOW_X, ALARM_WINDOW_Y, ALARM_WINDOW_W, ALARM_WINDOW_H, GxEPD_WHITE);
    } while (_display.nextPage());
  }



  void displayAlarmTime(Alarm *alarm) {

    PrintString valueString;
    setFont(&FreeMonoBold9pt7b);
    valueString.print(alarm->time().h(), DEC);
    valueString.print(':');
    if (alarm->time().m() < 10) valueString.print("0");
    valueString.print(alarm->time().m(), DEC);
    valueString.print(" ");
    valueString.print(alarm->volume());
    valueString.print(" ");
    valueString.print(alarm->tone());
    

    _display.setPartialWindow(ALARM_WINDOW_X, ALARM_WINDOW_Y, ALARM_WINDOW_W, ALARM_WINDOW_H);
    _display.firstPage();
    do {
      _display.setCursor(ALARM_CURSOR_X, ALARM_CURSOR_Y);
      _display.print(valueString);
    } while (_display.nextPage());
  }

  void setBitmap(Bitmap &bitmap) {
    _display.setPartialWindow(BITMAP_X, BITMAP_Y, BITMAP_W, BITMAP_H);
    _display.firstPage();
    do {
      for (uint16_t x = 0; x < bitmap.w(); x++) {
        for (uint16_t y = 0; y < bitmap.h(); y++) {
          if (bitmap.bits()[y * bitmap.w() + x] == 0) {
            _display.drawPixel(296 - bitmap.w() + x, 1 + y, GxEPD_BLACK);
          }
        }
      }
    } while (_display.nextPage());
  }

  // Draw a line under the time to show it's being set
  void showSetTime() {
    clearSetAlarmTime();
    _display.setPartialWindow(TIME_UNDERLINE_X, TIME_UNDERLINE_Y, TIME_UNDERLINE_W, TIME_UNDERLINE_H);
    _display.firstPage();
    do {
      _display.fillRect(TIME_UNDERLINE_X, TIME_UNDERLINE_Y, TIME_UNDERLINE_W, TIME_UNDERLINE_H, GxEPD_BLACK);
    } while (_display.nextPage());
  }

  void showSetAlarmTime() {
    clearSetTime();
    _display.setPartialWindow(ALARM_UNDERLINE_X, ALARM_UNDERLINE_Y, ALARM_UNDERLINE_W, ALARM_UNDERLINE_H);
    _display.firstPage();
    do {
      _display.fillRect(ALARM_UNDERLINE_X, ALARM_UNDERLINE_Y, ALARM_UNDERLINE_W, ALARM_UNDERLINE_H, GxEPD_BLACK);
    } while (_display.nextPage());
  }

  void showSetAlarmTone() {
    clearSetTime();
    _display.setPartialWindow(ALARM_TONE_UNDERLINE_X, ALARM_TONE_UNDERLINE_Y, ALARM_TONE_UNDERLINE_W, ALARM_TONE_UNDERLINE_H);
    _display.firstPage();
    do {
      _display.fillRect(ALARM_TONE_UNDERLINE_X, ALARM_TONE_UNDERLINE_Y, ALARM_TONE_UNDERLINE_W, ALARM_TONE_UNDERLINE_H, GxEPD_BLACK);
    } while (_display.nextPage());
  }

  void clearSetTime() {
    _display.setPartialWindow(TIME_UNDERLINE_X, TIME_UNDERLINE_Y, TIME_UNDERLINE_W, TIME_UNDERLINE_H);
    _display.firstPage();
    do {
      _display.fillRect(TIME_UNDERLINE_X, TIME_UNDERLINE_Y, TIME_UNDERLINE_W, TIME_UNDERLINE_H, GxEPD_WHITE);
    } while (_display.nextPage());
  }

  void clearSetAlarmTime() {
    _display.setPartialWindow(ALARM_UNDERLINE_X, ALARM_UNDERLINE_Y, ALARM_UNDERLINE_W, ALARM_UNDERLINE_H);
    _display.firstPage();
    do {
      _display.fillRect(ALARM_UNDERLINE_X, ALARM_UNDERLINE_Y, ALARM_UNDERLINE_W, ALARM_UNDERLINE_H, GxEPD_WHITE);
    } while (_display.nextPage());
  }

  void clearSetAlarmTone() {
    _display.setPartialWindow(ALARM_TONE_UNDERLINE_X, ALARM_TONE_UNDERLINE_Y, ALARM_TONE_UNDERLINE_W, ALARM_TONE_UNDERLINE_H);
    _display.firstPage();
    do {
      _display.fillRect(ALARM_TONE_UNDERLINE_X, ALARM_TONE_UNDERLINE_Y, ALARM_TONE_UNDERLINE_W, ALARM_TONE_UNDERLINE_H, GxEPD_WHITE);
    } while (_display.nextPage());
  }

  void showAlarmSnoozing(time_t snoozeEpoch, time_t epoc, uint16_t snoozeDuration) {
    uint16_t i, x;

    uint16_t w = ((snoozeEpoch - epoc) * ALARM_SNOOZE_BAR_W) / snoozeDuration;

    _display.setPartialWindow(ALARM_SNOOZE_BAR_X, ALARM_SNOOZE_BAR_Y, ALARM_SNOOZE_BAR_W, ALARM_SNOOZE_BAR_H);
    _display.firstPage();
    do {
      for (i = 0; i < w / (2 * ALARM_SNOOZE_BAR_B); i++) {
        x = ALARM_SNOOZE_BAR_X + 2 * ALARM_SNOOZE_BAR_B * i;
        _display.fillRect(x, ALARM_SNOOZE_BAR_Y, ALARM_SNOOZE_BAR_B, ALARM_SNOOZE_BAR_B, GxEPD_BLACK);
      }
    } while (_display.nextPage());
  }

  void clearAlarmSnoozing() {
    _display.setPartialWindow(ALARM_SNOOZE_BAR_X, ALARM_SNOOZE_BAR_Y, ALARM_SNOOZE_BAR_W, ALARM_SNOOZE_BAR_H);
    _display.firstPage();
    do {
      _display.fillRect(ALARM_SNOOZE_BAR_X, ALARM_SNOOZE_BAR_Y, ALARM_SNOOZE_BAR_W, ALARM_SNOOZE_BAR_H, GxEPD_WHITE);
    } while (_display.nextPage());
  }

protected:

  GxEPD2_BW<GxEPD2_290_T94_V2, 128> &_display;
  const GFXfont *_font;
};
#endif