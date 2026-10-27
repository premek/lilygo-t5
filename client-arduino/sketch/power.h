#pragma once
#include <Arduino.h>

#include "./config.h"

// Sleeps for SLEEP_SECONDS, then the board restarts from setup().
[[noreturn]] inline void deepSleep(const uint64_t sleepSeconds) {
  esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000ULL);
  Serial.print("Going to sleep now for seconds: ");
  Serial.println(sleepSeconds);
  Serial.flush();
  esp_deep_sleep_start();
}
