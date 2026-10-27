// Copy this file to config.h and fill in your values.
#pragma once
#include <Arduino.h>

#include "./wifi_credential.h"

constexpr WifiCredential WIFI_NETWORKS[] = {
    {"wifi-ssid", "CHANGE_ME"},
    {"some-fallback-ssid", "CHANGE_ME"},
};

constexpr const char* WEATHER_HOST = "192.168.0.999";
constexpr uint16_t WEATHER_PORT = 9000;

constexpr const char* LAT = "45.7821578";
constexpr const char* LON = "4.8481081";
constexpr const char* ALT = "161";

const String HEALTHCHECKS_IO_MONITOR_BASE_PATH = "/AaaAa1AaaAaXxxXxxxXxxx/eink";
