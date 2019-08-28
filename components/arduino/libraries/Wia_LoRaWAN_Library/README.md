# Wia LoRaWAN


## Overview
Wia LoRaWAN module was designed for internal company use for quick implementation with Wia Dot One.
A separate library, LMIC, has been wrapped and simplified for use with this board.


## Environment Setup
- Pull this repository.
- Move LMIC_ESP32 folder to arduino/libraries.
- Attach external antenna to LoRaWAN module.
- Mount LoRaWAN module on Dot One.


## Key configuration
The keys need to be passed during initialisation. The below examples are based on values received from LoRa Server dashboard.

A **DEV EUI** key such as dec166d3f6567c65 would be entered as:
{ 0x65, 0x7C, 0x56, 0xF6, 0xD3, 0x66, 0xC1, 0xDE }

An **APP key** such as e2688a50f94f31c79bb2e8db0501ec95 would be entered as:
{ 0xE2, 0x68, 0x8A, 0x50, 0xF9, 0x4F, 0x31, 0xC7, 0x9B, 0xB2, 0xE8, 0xDB, 0x05, 0x01, 0xEC, 0x95 }


## Reference
Open sample1 with Arduino IDE.

### void init(byte N_APPEUI[8], byte N_DEVEUI[8], byte N_APPKEY[16]);
Initialises the library and serial communication. Keys need to be formed in specific order, see above. Examples given in sample code section.

### bool device.( String packet, int timeout=15000 );
Adds a string message to the queue. If there was an error, such as transmitter busy or message oversize, this will return false.
Otherwise the function will return upon TX complete event or timeout in milliseconds.

### void device.loop();
Must be looped continuously. There cannot be delays in the program as the library will loose sync with the transmitter.

### void device.checkWakeupReason();
Print the reason for waking up

### void device.printNetworkStatus();
Print information about the network status.

### bool device.didTXcomplete();
Return whether an TX complete is just after coming in.

## Sample Code 1

Simple sending message via LoRaWAN.

    #include "Wia_LoRaWAN_Library.h"


    Wia_LoRaWAN device;



    void setup() {

      byte AppEUI[8] = { 0x65, 0xF7, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
      byte DevEUI[8] = { 0x65, 0x7C, 0x56, 0xF6, 0xD3, 0x66, 0xC1, 0xDE };
      byte AppKey[16] = { 0xE2, 0x68, 0x8A, 0x50, 0xF9, 0x4F, 0x31, 0xC7, 0x9B, 0xB2, 0xE8, 0xDB, 0x05, 0x01, 0xEC, 0x95};

      device.init(AppEUI, DevEUI, AppKey);

      device.sendMessage("First message");

    }

    void loop() {

      device.loop();

    }


## Sample Code 2

Sending message every 30 seconds and going to deep sleep.

    #include "Wia_LoRaWAN_Library.h"
    #define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */


    Wia_LoRaWAN device;
    unsigned long wakeTime = millis();

    // persistent variable
    RTC_DATA_ATTR bool firstMessageSent = false;


    void setup() {

      byte AppEUI[8] = { 0x65, 0xF7, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
      byte DevEUI[8] = { 0x65, 0x7C, 0x56, 0xF6, 0xD3, 0x66, 0xC1, 0xDE };
      byte AppKey[16] = { 0xE2, 0x68, 0x8A, 0x50, 0xF9, 0x4F, 0x31, 0xC7, 0x9B, 0xB2, 0xE8, 0xDB, 0x05, 0x01, 0xEC, 0x95};

      device.init(AppEUI, DevEUI, AppKey);

      if( !firstMessageSent ){
        device.sendMessage("First message!!");
      } else {
        device.sendMessage("Woke up from sleep.");
      }

    }

    void loop() {

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
    
## Related material

Conclusions on Alltech Project involving LoRa:

https://docs.google.com/document/d/1vsVZtvaaxKZ3NN3zMBP45Hjw9cUMUk5e-Q4uiSZ9HYQ/edit?usp=sharing
