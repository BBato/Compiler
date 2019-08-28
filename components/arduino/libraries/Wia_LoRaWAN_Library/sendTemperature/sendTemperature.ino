#include "Wia_LoRaWAN_Library.h"
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */

#include "DHT.h"

#define DHTPIN 22     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

Wia_LoRaWAN device;
unsigned long wakeTime = millis();

// persistent variable
RTC_DATA_ATTR bool firstMessageSent = false;

byte AppEUI[8] = { 0x65, 0xF7, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
byte DevEUI[8] = { 0x65, 0x7C, 0x56, 0xF6, 0xD3, 0x66, 0xC1, 0xDE };
byte AppKey[16] = { 0xE2, 0x68, 0x8A, 0x50, 0xF9, 0x4F, 0x31, 0xC7, 0x9B, 0xB2, 0xE8, 0xDB, 0x05, 0x01, 0xEC, 0x95};


void setup() {

  pinMode(26,INPUT); //Setting GPIO pin as input pin
  pinMode(13,INPUT); //Setting GPIO pin as input pin

  dht.begin();

  device.init(AppEUI, DevEUI, AppKey);
  
}

void loop() {

  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if(isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    // go to sleep early
    goSleep();
  }

  device.sendMessage(String(t));
  Serial.println(t);

  // remain active for 30 seconds
  while( wakeTime + 30000 > millis() ){

    // do work
    device.loop();

    // check for transmission complete?
    if( device.didTXcomplete() ){

      Serial.println("Received TX complete event");
      
      // go to sleep early
      goSleep();
    }
    
  }

  // sleep for 30 seconds
  goSleep();
  
}


void goSleep() {

  // enable timer interrupt
  esp_sleep_enable_timer_wakeup(30 * uS_TO_S_FACTOR);

  // deep sleep wakeup will restart the program.
  esp_deep_sleep_start();
  
}
