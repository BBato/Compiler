#include "Wia.h"
#include <Arduino.h>
#include <HTTPClient.h>
#include <Preferences.h>

Wia::Wia() {}

Wia::Wia(String access_token) {
  _access_token = access_token;
}

String Wia::getAccessToken() {
  if (_access_token == NULL) {
    Preferences preferences;

    preferences.begin("storage", true);
    _access_token = preferences.getString("secret_key");

    preferences.end();
  }

  return _access_token;
}

String Wia::getCurrentDeploymentId() {
  Preferences preferences;

  preferences.begin("storage", true);
  String current_deployment_id = preferences.getString("deployment_id");

  preferences.end();

  return current_deployment_id;
}

int Wia::createEvent(String name) {
  return createEvent(name, "");
}

int Wia::createEvent(String name, int data) {
  return createEvent(name, String(data));
}

int Wia::createEvent(String name, double data) {
  return createEvent(name, String(data));
}

int Wia::createEvent(char *name, int data) {
  return createEvent(String(name), String(data));
}

int Wia::createEvent(char *name, double data) {
  return createEvent(String(name), String(data));
}

int Wia::createEvent(char *name,  char *data) {
  return createEvent(String(name), String(data));
}

int Wia::createEvent(String name, String data) {
  HTTPClient http;

  String body = "{\"name\":\"" + name +"\",\"data\":\""+ data + "\"}";

  http.begin("http://api.wia.io/v1/events");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + getAccessToken() );
  http.addHeader("Content-Length", String(body.length()));

  int httpCode =  http.POST(body);

  if(httpCode > 0) {
      // USE_SERIAL.printf("[HTTP] POST... code: %d\n", httpCode);

      if(httpCode == HTTP_CODE_OK) {
          String payload = http.getString();
          // USE_SERIAL.println(payload);
      }
  } else {
      // USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return httpCode;
}

String Wia::getLatestDeploymentId() {
  HTTPClient http;

  http.begin("http://api.wia.io/v1/firmware/latestDeploymentId");
  http.addHeader("Content-Type", "text/plain");
  http.addHeader("Authorization", "Bearer " + getAccessToken() );

  int httpCode =  http.GET();

  String result = "";

  if (httpCode == HTTP_CODE_OK) {
    result = http.getString();
  } else {
    // USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return result;
}

String Wia::getDeviceState(char *key) {
  return getDeviceState(String(key));
}

String Wia::getDeviceState(String key) {
  HTTPClient http;

  http.begin("http://api.wia.io/v1/devices/me/state/" + key);
  http.addHeader("Content-Type", "text/plain");
  http.addHeader("Authorization", "Bearer " + getAccessToken() );

  int httpCode =  http.GET();

  String result = "";

  if (httpCode == HTTP_CODE_OK) {
    result = http.getString();
  } else {
    // USE_SERIAL.printf("[HTTP] POST... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return result;
}
