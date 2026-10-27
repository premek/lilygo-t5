#pragma once
#include <Arduino.h>
#include <HttpClient.h>

#include <vector>

constexpr int HTTP_200_OK = 200;
constexpr int HTTP_408_REQUEST_TIMEOUT = 408;
constexpr int HTTP_429_TOO_MANY_REQUESTS = 429;
constexpr int HTTP_500_INTERNAL_SERVER_ERROR = 500;
constexpr int HTTP_502_BAD_GATEWAY = 502;
constexpr int HTTP_503_SERVICE_UNAVAILABLE = 503;
constexpr int HTTP_504_GATEWAY_TIMEOUT = 504;

class Http {
 public:
  Http(Client& client,  // NOLINT(runtime/references)
       const char* host, uint16_t port)
      : http_(client), host_(host), port_(port) {}

  ~Http() { http_.stop(); }

  // GET, ignoring the response body. Returns the HTTP status or a negative error code.
  int getStatus(const String& path) {
    int status = sendWithRetry(path);
    http_.stop();
    return status;
  }

  // GET and return the whole response body.
  // Empty on failure. May be shorter than the full body if the connection dropped
  std::vector<uint8_t> getBody(const String& path) {
    int status = sendWithRetry(path);
    Serial.print("status code: ");
    Serial.println(status);
    std::vector<uint8_t> body;
    if (status == HTTP_200_OK) body = readBody();
    http_.stop();
    return body;
  }

 private:
  static bool isRetryable(int status) {
    if (status < 0) return true;  // transport error
    switch (status) {
      case HTTP_408_REQUEST_TIMEOUT:
      case HTTP_429_TOO_MANY_REQUESTS:
      case HTTP_500_INTERNAL_SERVER_ERROR:
      case HTTP_502_BAD_GATEWAY:
      case HTTP_503_SERVICE_UNAVAILABLE:
      case HTTP_504_GATEWAY_TIMEOUT:
        return true;
      default:
        return false;
    }
  }

  // Sends the request, retrying on transport errors and retryable statuses.
  // On a non-retryable result the connection stays open for reading.
  int sendWithRetry(const String& path) {
    int status = -1;
    for (int i = 0; i < RETRY_ATTEMPTS; i++) {
      if (i > 0) delay(RETRY_DELAY_MILLIS);
      status = attempt(path);
      if (!isRetryable(status)) return status;
      http_.stop();
    }
    return status;
  }

  int attempt(const String& path) {
    Serial.print("Connecting to ");
    Serial.println(host_);

    int err = http_.get(host_, port_, path.c_str());
    if (err != 0) {
      Serial.print("Connect failed: ");
      Serial.println(err);
      return -1;
    }
    int status = http_.responseStatusCode();
    if (status < 0) {
      Serial.print("Getting response failed: ");
      Serial.println(status);
    }
    return status;
  }

  // Skips headers, then reads until the server closes the connection or no data
  // arrives for BODY_TIMEOUT_MILLIS. The timeout is per chunk, so total time is unlimited.
  std::vector<uint8_t> readBody() {
    std::vector<uint8_t> body;
    int err = http_.skipResponseHeaders();
    if (err < 0) {
      Serial.print("Failed to skip response headers: ");
      Serial.println(err);
      return body;
    }

    uint8_t chunk[CHUNK_BYTES];
    uint32_t lastData = millis();
    while ((http_.connected() || http_.available()) &&
           (millis() - lastData) < BODY_TIMEOUT_MILLIS) {
      int n = http_.read(chunk, sizeof(chunk));
      if (n > 0) {
        if (body.size() + n > MAX_BODY_BYTES) {
          Serial.println("Response body too large");
          body.clear();
          return body;
        }
        body.insert(body.end(), chunk, chunk + n);
        lastData = millis();
      } else {
        delay(BODY_POLL_MILLIS);
      }
    }
    return body;
  }

  static constexpr int RETRY_ATTEMPTS = 3;
  static constexpr uint32_t RETRY_DELAY_MILLIS = 3000;

  static constexpr size_t MAX_BODY_BYTES = 64 * 1024;

  static constexpr size_t CHUNK_BYTES = 128;
  static constexpr uint32_t BODY_TIMEOUT_MILLIS = 30 * 1000;
  static constexpr uint32_t BODY_POLL_MILLIS = 100;

  HttpClient http_;
  const char* host_;
  uint16_t port_;
};
