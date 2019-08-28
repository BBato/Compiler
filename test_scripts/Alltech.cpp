#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "Arduino.h"
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <stdlib.h>
#include <WiFi.h>

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define PIR_SENSOR 25
#define RFID_ENABLE 22
#define RFID_SCAN_INTERVAL  30000
#define INITIAL_DELAY       60000
#define INT_TIMER_SETTING 30
#define MAX_TAGS_PER_MESSAGE 4

void read_tags();
void send_LORA_data();
void send_initial_LORA_data();


void loopTask(void *pvParameters)
{
    setup();
    for(;;) {
        micros(); //update overflow
        loop();
    }
}

extern "C" void app_main()
{
    initArduino();
    xTaskCreatePinnedToCore(loopTask, "loopTask", 8192, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
}


RTC_DATA_ATTR int state = 0;
// 0 - Initial state. Device sends an initial message to show working mode and waits 1 minute.
// 1 - PIR interrupt detected, starting RFID scan
// 2 - RFID scan for 30 seconds
// 3 - Ending RFID scan
// 4 - Sending collected data (optional)
// 5 - Going to sleep to wake in 30 seconds
// 6 - Woke up to enable PIR sensor and go back to sleep until PIR interrupts

String messageDraft = "";
int tagsInMessageDraft = 0;
unsigned int empcounter = 0;
boolean tx_attempted = false;
static osjob_t sendjob;
bool initial_message_sent = false;

unsigned long RFID_scan_start_time = millis();
unsigned long initial_delay_time = millis();


// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.

static const u1_t PROGMEM APPEUI[8] = { 0x65, 0xF7, 0x00, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) {
 memcpy_P(buf, APPEUI, 8);
}

static const u1_t PROGMEM DEVEUI[8] = { 0x22,  0x75,  0xA5,  0xCF,  0x12,  0xA2,  0x52, 0x5A }; //{ 0xDE,0xC1,0x66,0xD3,0xF6,0x56,0x7C,0x65 };
void os_getDevEui (u1_t* buf) {
 memcpy_P(buf, DEVEUI, 8);
}

static const u1_t PROGMEM APPKEY[16] = { 0xA9, 0x68, 0x8A, 0x50, 0xF9, 0x4F, 0x31, 0xC7, 0x9B, 0xB2, 0xE8, 0xDB, 0x05, 0x01, 0xEC, 0x85 };
void os_getDevKey (u1_t* buf) {
 memcpy_P(buf, APPKEY, 16);
}

const lmic_pinmap lmic_pins = {
  .nss = 5,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 14,
  .dio = {26, 21, LMIC_UNUSED_PIN},
};


void onEvent (ev_t ev) {
  
  Serial.print(os_getTime());
  Serial.print(": ");
  
  switch (ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
      
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
      
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
      
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
      
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
      
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      //displayNetworkInformation();
      LMIC_setLinkCheckMode(0);
      break;
      
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
      
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
      
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE"));
      state = 5;
      break;
      
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
      
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
      
    case EV_RXCOMPLETE:
      Serial.println(F("EV_RXCOMPLETE"));
      break;
      
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
      
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
      
    case EV_TXSTART:
      Serial.println(F("EV_TXSTART"));
      break;
      
    default:
      Serial.print(F("Unknown event: "));
      Serial.println((unsigned) ev);
      break;
  }
  
}


void loop() {
  
  switch(state){
    case 0:
      if( !initial_message_sent ){
        initial_message_sent = true;
        Serial.println("Sending an initial join message..");
        send_initial_LORA_data();
      }
      if( millis() > initial_delay_time + INITIAL_DELAY ){
        state = 1;
        Serial.println("Overtime.");
      }
      break;
    case 1:                                                               // State 1- Start RFID scan
      Serial.println("State 1: Starting RFID scan...");
      RFID_scan_start_time = millis();
      digitalWrite(RFID_ENABLE, HIGH);
      messageDraft = "";
      tagsInMessageDraft = 0;
      state = 2;
      // don't break; state 2 follows.
      
    case 2:                                                               // State 2- Scan RFID
      if( millis() > RFID_scan_start_time + RFID_SCAN_INTERVAL ){
        state = 3;
      }
      read_tags();
      break;
      
    case 3:                                                               // State 3- End RFID scan
      Serial.println("State 3: Finishing RFID scan.");
      digitalWrite(RFID_ENABLE, LOW);
      tx_attempted = false;
      if( tagsInMessageDraft ){
        state=4;
        Serial.println("State 4: Sending LoRa data.....");     
      } else {
        state=5;
        Serial.println("There were no tags to be sent. Jumping to state 5.");
      }
      break;
      
    case 4:                                                               // State 4- Send any data
      if( tx_attempted == false ){
        send_LORA_data();
        tx_attempted = true;
      }
      break;
      
    case 5:                                                               // State 5- Going to sleep after work
      Serial.println("State 5: Going to sleep with timer interrupt.");
      //digitalWrite(lmic_pins.nss, LOW); // NSS
      LMIC_shutdown();
      esp_sleep_enable_timer_wakeup(INT_TIMER_SETTING * uS_TO_S_FACTOR);
      esp_deep_sleep_start();
      // no need to break as sleep will never return here.
      
    default:
      // Error. Go to sleep and restart states.
      Serial.println("Fatal error. No state.");
      state = 1;
      esp_sleep_enable_timer_wakeup(INT_TIMER_SETTING * uS_TO_S_FACTOR);
      esp_deep_sleep_start();      
      break;
  }
  
  if (Serial.available() > 0) {
    char schar = Serial.read();
    Serial.println("Version: Aug 8 2019");
  }
  
  os_runloop_once();
  
}


void collectTag(String tag) {
  
  // discard corrupted tags
  if( tag == "000000000000000000" ){
    Serial.println("Received a corrupted tag reading.");
    return;
  }
  
  if( tagsInMessageDraft > MAX_TAGS_PER_MESSAGE-1 ){
    Serial.print("Already storing too many tags to send! Ignoring new tag: ");
    Serial.println(tag);
    return;
  }
  
  if( messageDraft.indexOf(tag) == -1 ){
    messageDraft += " ";
    messageDraft += tag;
    Serial.print("Updated message draft: ");
    Serial.println(messageDraft);
    tagsInMessageDraft++;
    Serial.print("New amount of tags in message draft: ");
    Serial.println(tagsInMessageDraft);
  } else {
    Serial.println("The message draft already contained this tag.");
  }
   
}


void send_LORA_data() {
  
  Serial.println(F("send_LORA_data()"));
  uint8_t mydata[messageDraft.length()];
  
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
      Serial.println(F("OP_TXRXPEND, not sending!"));
  } else {
    for (int i = 0; i < messageDraft.length(); i++) {
      mydata[i] = messageDraft[i];
    }
    
    LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
    messageDraft = "";
    tagsInMessageDraft = 0;
    Serial.println(F("LMIC_setTxData2(...)"));
  }
  
}


void send_initial_LORA_data() {
  
  Serial.println(F("send_initial_LORA_data()"));
  String initialMessage = "Joining!";
  uint8_t mydata[initialMessage.length()];
  
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
      Serial.println(F("OP_TXRXPEND, not sending!"));
  } else {
    for (int i = 0; i < initialMessage.length(); i++) {
      mydata[i] = initialMessage[i];
    }
    
    LMIC_setTxData2(1, mydata, sizeof(mydata), 0);
    messageDraft = "";
    tagsInMessageDraft = 0;
    Serial.println(F("LMIC_setTxData2"));
  }
  
}


String decodeHDX(String str) {
  
  Serial.println("decodeHDX()");
  String strr, tagData;
  for (int i = 0; i <= str.length(); i++) {
    strr += str.charAt(str.length() - i);
  }
  char CC[4];
  strr.substring(14, 17).toCharArray(CC, 4);
  char ID[11];
  strr.substring(17).toCharArray(ID, 11);
  long cc = strtol(CC, NULL, 16);
  if (cc != 0) {
    //Serial.println(cc);
    long id = strtol(ID, NULL, 16);
    //Serial.println(id);
    String idp = String(id);
    while (idp.length() < 12) {
      idp = "0" + idp;
    }
    tagData = String(cc) + idp;
  } else {
    char tmp[27];
    strr.substring(1).toCharArray(tmp, 27);
    long id = strtol(tmp, NULL, 16);
    String idp = String(id);
    while (idp.length() < 18) {
      idp = "0" + idp;
    }
    tagData = idp;
  }
  return tagData;
  
}


void read_tags() {
  
  String tag = "";
  while (Serial2.available()) {
    tag += (char)Serial2.read();
  }
  if (tag != "") {
    tag.remove(27);
    String dtag = decodeHDX(tag);
    Serial.print("Processing new tag: ");
    Serial.println(dtag);
    collectTag(dtag);
  }
  
}


void wakeup_reason() {
  
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 :
      Serial.println("Woke from deep sleep because of PIR.");
      Serial.println("Disabling PIR sensor interrupt.");
      //esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_EXT0);
      state = 1;      
      break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer");
      state = 6;
      Serial.println("State 6.");
      Serial.println("Woke from deep sleep because of timer.");
      Serial.println("Enabling PIR sensor interrupt and returning to sleep.");
      esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 1);
      esp_deep_sleep_start();
      break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;    
  }
  
}


void displayNetworkInformation() {
  
  u4_t netid = 0;
  devaddr_t devaddr = 0;
  u1_t nwkKey[16];
  u1_t artKey[16];
  LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
  Serial.print("netid: ");
  Serial.println(netid, DEC);
  Serial.print("devaddr: ");
  Serial.println(devaddr, HEX);
  Serial.print("artKey: ");
  for (int i = 0; i < sizeof(artKey); ++i) {
    if (i != 0)
      Serial.print("-");
    Serial.print(artKey[i], HEX);
  }
  Serial.println("");
  Serial.print("nwkKey: ");
  for (int i = 0; i < sizeof(nwkKey); ++i) {
    if (i != 0)
      Serial.print("-");
    Serial.print(nwkKey[i], HEX);
  }
  Serial.println("");
  
}


void setup() {
  
  Serial.begin(115200);
  delay(100);
  Serial.println("setup()");
  Serial2.begin(9600);
  delay(100);
  pinMode(RFID_ENABLE, OUTPUT);
  digitalWrite(RFID_ENABLE, HIGH);
  pinMode(PIR_SENSOR, INPUT);
  WiFi.mode(WIFI_OFF);
    
  os_init();
  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 20 / 100);
  LMIC_setDrTxpow(DR_SF7, 10);
  LMIC_selectSubBand(1);
  Serial.println("Checking wakeup reason?");
  wakeup_reason();

}