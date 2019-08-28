/*
  Arduino Library for the Wia LoRaWAN shield

  Development environment specifics:
  Arduino IDE 1.8.8
  Microsoft Visual Studio Code
  Wia LoRaWAN module

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.
  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Made by Wia Technologies - find us at wia.io .
*/


#include <Wia_LoRaWAN_Library.h>

#include <lmic.h>
#include <hal/hal.h>

static u1_t PROGMEM APPEUI[8];
void os_getArtEui (u1_t* buf) {
 memcpy_P(buf, APPEUI, 8);
}

static u1_t PROGMEM DEVEUI[8];
void os_getDevEui (u1_t* buf) {
 memcpy_P(buf, DEVEUI, 8);
}

static u1_t PROGMEM APPKEY[16];
void os_getDevKey (u1_t* buf) {
 memcpy_P(buf, APPKEY, 16);
}

const lmic_pinmap lmic_pins = {
  .nss = 32,
  .rxtx = LMIC_UNUSED_PIN,
  .rst = 12,
  .dio = {14, 26, LMIC_UNUSED_PIN},
};

bool local_LoRaWAN_TXcomplete = false;

void Wia_LoRaWAN::checkWakeupReason() {
  esp_sleep_wakeup_cause_t wakeup_reason;
  wakeup_reason = esp_sleep_get_wakeup_cause();
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_EXT0 :  Serial.println("Wakeup caused by external interrupt 0."); break;
    case ESP_SLEEP_WAKEUP_EXT1 :  Serial.println("Wakeup caused by external interrupt 1."); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer."); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason); break;    
  }
}


Wia_LoRaWAN::Wia_LoRaWAN(){}


void Wia_LoRaWAN::init(byte N_APPEUI[8], byte N_DEVEUI[8], byte N_APPKEY[16]){

  // transfer the keys from the initialiser

  for( int i=0; i<8; i++ ){
    APPEUI[i] = N_APPEUI[i];
    DEVEUI[i] = N_DEVEUI[i];
    APPKEY[i] = N_APPKEY[i];
    APPKEY[i+8] = N_APPKEY[i+8];
  }

  // continue boot procedure.

  delay(200);
  Serial.println("os_init();");
  os_init();
  Serial.println("end.");

  LMIC_reset();
  LMIC_setClockError(MAX_CLOCK_ERROR * 20 / 100);
  LMIC_setDrTxpow(DR_SF7, 10);
  LMIC_selectSubBand(1);
  Serial.println("Checking wakeup reason?");
  checkWakeupReason();


  Serial.println("Finished.");

}


void Wia_LoRaWAN::loop(){
  os_runloop_once();
}


bool Wia_LoRaWAN::sendMessage( String packet, int timeout ) {
  
  Serial.println(F("sendMessage()"));
  uint8_t outbox[packet.length()];
  
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
      Serial.println(F("Transmitter busy, try again later!"));
      return false;
  } else {
    for (int i = 0; i < packet.length(); i++) {
      outbox[i] = packet[i];
    }
    
    LMIC_setTxData2(1, outbox, sizeof(outbox), 0);
    Serial.println(F("Transmitter queued message."));
    transmissionTime = millis();
  }

  while( true ){
    loop();
    if( local_LoRaWAN_TXcomplete ){
      Serial.print("Transmitter got tx complete. Returning. It took ");
      Serial.println( String( millis() - transmissionTime ));
      break;
    } else if ( millis() > transmissionTime + timeout ){
      Serial.println("Transmitter timed out. Returning.");
      break;
    }
  }

  return true;
  
}


void Wia_LoRaWAN::printNetworkStatus() {
  
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


bool Wia_LoRaWAN::didTXcomplete(){
  if( local_LoRaWAN_TXcomplete ){
    local_LoRaWAN_TXcomplete = false;
    return true;
  }
  return false;
}


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
      local_LoRaWAN_TXcomplete = true;
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