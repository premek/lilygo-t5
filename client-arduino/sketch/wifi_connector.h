#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>

#include "./wifi_credential.h"

class WifiConnector {
 public:
  template <size_t N>
  explicit WifiConnector(const WifiCredential (&networks)[N]) {
    for (const auto& n : networks) multi_.addAP(n.ssid, n.pass);
  }

  bool connect() {
    Serial.print("Waiting for WiFi...");
    uint32_t start = millis();

    while (multi_.run() != WL_CONNECTED) {
      if (millis() - start > CONNECT_TIMEOUT_MILLIS) {
        Serial.println();
        Serial.println("WiFi timeout");
        return false;
      }
      Serial.print(".");
      delay(POLL_DELAY_MILLIS);
    }
    Serial.println();
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
    return true;
  }

 private:
  static constexpr uint32_t CONNECT_TIMEOUT_MILLIS = 30 * 1000;
  static constexpr uint32_t POLL_DELAY_MILLIS = 150;

  WiFiMulti multi_;
};
