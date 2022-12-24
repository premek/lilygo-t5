
// HttpClient 2.2.0 by Adrian McEwen
// Adafruit GFX Library by Adafruit (install with dependencies)
// https://github.com/michaelkamprath/ePaperDriverLib (Code -> Download ZIP -> extract to ~/Arduino/libraries)

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HttpClient.h>
#include <ePaperDriver.h>

#define SLEEP_SECONDS 60*15

#define ELINK_SS (5)
#define ELINK_BUSY (4)
#define ELINK_RESET (16)
#define ELINK_DC (17)

#define BUTTON_PIN (39)


char* pass = "your_wifi_pass";
char* ssid = "mom click here for internet";

const String lat = "25.276987";
const String lon = "55.296249";
const String alt = "5";

const uint16_t port = 1337;
const char* host = "192.168.0.75";
String path = "/weather?format=eink&lat="+lat+"&lon="+lon+"&alt="+alt;

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30 * 1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;


const int screenBytes = 2756;  // bit per pixel: 212x104 / 8;

// two bitmaps combined determines the pixel color: both 1 = black, both 0 = white, 1/0 = gray1, 0/1 = gray2
uint8_t blackBitMap[screenBytes] = { 0 };
uint8_t colorBitMap[screenBytes] = { 0 };

boolean downloaded = false;

void setup() {
  Serial.begin(115200);

  wifi_connect();
  download_bitmap();
  display_bitmap();
  deep_sleep();
}


void wifi_connect() {
  WiFiMulti WiFiMulti;

  WiFiMulti.addAP(ssid, pass);

  Serial.println();
  Serial.print("Waiting for WiFi...");

  while (WiFiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(150);
  }

  Serial.println("");
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
  //delay(500);
}

void download_bitmap() {
  Serial.print("Connecting to ");
  Serial.println(host);

  WiFiClient client;
  HttpClient http(client);
  get_bitmap(http);
  http.stop();
}

void get_bitmap(HttpClient http) {
  int get_err = http.get(host, port, path.c_str());
  if (get_err != 0) {
    Serial.print("Connect failed: "); // TODO show errors on screen
    Serial.println(get_err);
    return;
  }

  int status = http.responseStatusCode();
  if (status < 0) {
    Serial.print("Getting response failed: ");
    Serial.println(status);
    return;
  }

  Serial.print("status code: ");
  Serial.println(status);

  int skip_err = http.skipResponseHeaders();
  if (skip_err < 0) {
    Serial.print("Failed to skip response headers: ");
    Serial.println(skip_err);
    return;
  }

  unsigned long timeoutStart = millis();
  int i = 0;

  // Whilst we haven't timed out & haven't reached the end of the body
  while ((http.connected() || http.available()) && ((millis() - timeoutStart) < kNetworkTimeout)) {
    if (!http.available()) {
      delay(kNetworkDelay);
      continue;
    }

    char c = http.read();

    // response should be 2x screenBytes long.
    // the first half is the 'black' bitmap and the second half is the 'color' bitmap
    if (i < screenBytes) {
      blackBitMap[i] = c;
    } else {
      colorBitMap[i - screenBytes] = c;
    }

    i++;

    // We read something, reset the timeout counter
    timeoutStart = millis();
  }

  downloaded = true;
}


void display_bitmap() {
  if (!downloaded) {
    return;
  }

  ePaperDisplay* device = new ePaperDisplay(GDEW0213T5, ELINK_BUSY, ELINK_RESET, ELINK_DC, ELINK_SS);

  device->setDeviceImage(
    blackBitMap,
    screenBytes,  //blackBitMapSize
    false,        //blackBitMapIsProgMem

    colorBitMap,
    screenBytes,  //colorBitMapSize
    false);       //colorBitMapIsProgMem

  device->refreshDisplay();
}

void deep_sleep() {
  esp_sleep_enable_timer_wakeup(SLEEP_SECONDS * 1000000ULL);
  //  esp_sleep_enable_ext0_wakeup((gpio_num_t)BUTTON_PIN, LOW);
  Serial.println("Going to sleep now");
  Serial.flush();
  esp_deep_sleep_start();
}

void loop() {
}
