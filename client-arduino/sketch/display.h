#pragma once
#include <Adafruit_GFX.h>
#include <Arduino.h>
#include <Fonts/FreeSans9pt7b.h>
#include <ePaperDriver.h>

#include "./bitmap.h"

class Display {
 public:
  Display(uint8_t busy, uint8_t reset, uint8_t dc, uint8_t ss)
      : dev_(GDEW0213T5, busy, reset, dc, ss) {}

  bool show(DualBitmap& bitmap) {
    if (!bitmap.loaded()) return false;
    dev_.setDeviceImage(bitmap.black(), bitmap.bytesPerPlane, false, bitmap.color(),
                        bitmap.bytesPerPlane, false);
    dev_.refreshDisplay();
    return true;
  }

  // Draws a small centered box with text near the top of the screen.
  void message(const char* text) {
    int16_t x, y;
    uint16_t w, h;
    dev_.setFont(&FreeSans9pt7b);
    dev_.getTextBounds(text, 0, 0, &x, &y, &w, &h);
    x = dev_.width() / 2 - w / 2;
    y = BOX_TOP;

    box(x - PAD_X, y - PAD_Y, w + 2 * PAD_X, h + 2 * PAD_Y);
    dev_.setTextColor(ePaper_BLACK);
    dev_.setCursor(x - 2, y + 14);
    dev_.print(text);
    dev_.refreshDisplay();
  }

 private:
  static constexpr int16_t BOX_TOP = 20;
  static constexpr int16_t PAD_X = 7;
  static constexpr int16_t PAD_Y = 5;

  void box(int16_t x, int16_t y, uint16_t w, uint16_t h) {
    dev_.fillRect(x, y, w, h, ePaper_WHITE);
    dev_.drawRect(x, y, w, h, ePaper_BLACK);
    dev_.drawRect(x - 1, y - 1, w + 2, h + 2,
                  ePaper_WHITE);  // workaround: border lines are too long
  }

  ePaperDisplay dev_;
};
