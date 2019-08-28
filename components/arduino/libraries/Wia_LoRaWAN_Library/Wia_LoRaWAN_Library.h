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


#ifndef WIA_LORA_LIBRARY_H
#define WIA_LORA_LIBRARY_H

#include <Arduino.h>


#include <lmic.h>
#include <hal/hal.h>


class Wia_LoRaWAN {

  public:
    Wia_LoRaWAN();
    void loop();
    void checkWakeupReason();
    bool sendMessage( String packet, int timeout=15000 );
    void printNetworkStatus();
    bool didTXcomplete();
    void init(u1_t PROGMEM N_APPEUI[8], u1_t PROGMEM N_DEVEUI[8], u1_t PROGMEM N_APPKEY[16]);

  private:
    unsigned long transmissionTime;     

};

#endif