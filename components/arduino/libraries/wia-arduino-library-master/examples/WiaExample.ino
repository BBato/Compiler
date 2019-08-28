#include <Wia.h>
#include <Arduino.h>
#include <WiFi.h>

const char* ssid     = ""; // Your WiFi ssid
const char* password = ""; //Your Wifi password
String access_token = ""; // Your device secret key

Wia wia = Wia(access_token);

void setup() {
   Serial.begin(115200);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
   }
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());


}

void loop() {
    wia.createEvent("hello-wia", "");
    delay(5000);
}
