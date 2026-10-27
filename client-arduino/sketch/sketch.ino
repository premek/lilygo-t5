// HttpClient 2.2.0 by Adrian McEwen
// Adafruit GFX Library by Adafruit (install with dependencies)
// https://github.com/michaelkamprath/ePaperDriverLib
//   Code -> Download ZIP -> extract to ~/Arduino/libraries
// UUID 0.1.6 by Rob Tillaart

#include "./bitmap.h"
#include "./config.h"
#include "./display.h"
#include "./http_helper.h"
#include "./monitor.h"
#include "./power.h"
#include "./wifi_connector.h"

constexpr uint64_t SLEEP_SECONDS_SUCCESS = 60 * 20;
constexpr uint64_t SLEEP_SECONDS_FAIL = 60 * 4;

constexpr uint16_t DISPLAY_WIDTH = 212;
constexpr uint16_t DISPLAY_HEIGHT = 104;

constexpr uint8_t ELINK_BUSY = 4;
constexpr uint8_t ELINK_RESET = 16;
constexpr uint8_t ELINK_DC = 17;
constexpr uint8_t ELINK_SS = 5;

WiFiClient client;
WifiConnector wifi(WIFI_NETWORKS);
Monitor monitor(client);
DualBitmap bitmap(DISPLAY_WIDTH, DISPLAY_HEIGHT);
Display display(ELINK_BUSY, ELINK_RESET, ELINK_DC, ELINK_SS);

void setup() {
  Serial.begin(115200);
  display.message("Refreshing");  // after wakeup the screen outside the message is not kept

  if (!wifi.connect()) {
    display.message("No WiFi");
    deepSleep(SLEEP_SECONDS_FAIL);
  }

  monitor.start();
  if (downloadBitmap()) {
    display.show(bitmap);
    monitor.success();
  } else {
    display.message("Download failed");
    monitor.fail();
  }

  deepSleep(SLEEP_SECONDS_SUCCESS);
}

void loop() {
  // never reached: deep sleep is entered in setup()
}

bool downloadBitmap() {
  Http http(client, WEATHER_HOST, WEATHER_PORT);
  bitmap.data =
      http.getBody(String("/weather?format=eink&lat=") + LAT + "&lon=" + LON + "&alt=" + ALT);
  return bitmap.loaded();
}
