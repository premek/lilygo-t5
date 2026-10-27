#pragma once
#include <Arduino.h>
#include <UUID.h>

#include "./config.h"
#include "./http_helper.h"

class Monitor {
 public:
  explicit Monitor(Client& client) : client_(client) {}

  void start() {
    uuid_.seed(esp_random(), esp_random());
    uuid_.generate();
    ping("/start");
  }
  void success() { ping(""); }
  void fail() { ping("/fail"); }

 private:
  void ping(const char* suffix) {
    Http http(client_, HOST, PORT);
    int status = http.getStatus(getPath(suffix));
    Serial.print("monitor response: ");
    Serial.println(status);
  }

  String getPath(const char* suffix) {
    return HEALTHCHECKS_IO_MONITOR_BASE_PATH + suffix + "?rid=" + uuid_.toCharArray();
  }

  static constexpr const char* HOST = "hc-ping.com";
  static constexpr uint16_t PORT = 80;

  Client& client_;
  UUID uuid_;
};
