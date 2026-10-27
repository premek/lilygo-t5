#pragma once
#include <Arduino.h>

#include <vector>

// Two 1-bit-per-pixel planes for a tri-colour e-paper display.
// Combined, they determine the pixel colour:
// both 1 = black, both 0 = white, 1/0 = gray1, 0/1 = gray2
// Layout: the 'black' plane followed by the 'color' plane.
struct DualBitmap {
  DualBitmap(uint16_t width, uint16_t height)
      : width(width),
        height(height),
        bytesPerPlane((width * height + 7) / 8),
        bothPlanesTotalBytes(2 * bytesPerPlane) {}

  const uint16_t width;
  const uint16_t height;
  const size_t bytesPerPlane;
  const size_t bothPlanesTotalBytes;

  std::vector<uint8_t> data;

  bool loaded() const { return data.size() == bothPlanesTotalBytes; }

  uint8_t* black() { return data.data(); }
  uint8_t* color() { return data.data() + bytesPerPlane; }
};
