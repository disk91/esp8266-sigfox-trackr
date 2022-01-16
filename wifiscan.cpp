/* ======================================================================
    This file is part of disk91_wifi.

    disk91_wifi is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
  =======================================================================
*/
/* ======================================================================
 *  ESP8266 / WiFi Scanner /
 * ----------------------------------------------------------------------
 * (c) Disk91 - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */
#include "wifiscan.h"
#include "ESP8266WiFi.h"

WifiScanClass wifiscanService;

#define WIFISCAN_SSIDFILTERLEN  5
static const char  ssidFiltered[WIFISCAN_SSIDFILTERLEN][16]= {
  "android",
  "phone",
  "samsung",
  "huawei",
  "tp-dis" 
};

/**
 * Start a WiFi scan sequence for the given timeoutMs duration (step of 2180 Ms)
 * The scan also stop if the maxAp number of Access Point have been discovered
 * With filtered option the MAC list will be filtered using some basic rules to
 * ensure a better sucess for geolocation
 * - Only unicast
 * - Public SSID not containg keyworks like ( android, phone, samsung, huawei ) for ermoving mobile hotspot
 * - Full 00 
 * - Locally administred ( byte 0, bit 1) = 1
 */
void WifiScanClass::startScan(uint32_t timeoutMs, uint8_t maxAp, boolean filtered) {

    // Init WiFi from sleep mode
    WiFi.forceSleepWake();
    WiFi.mode(WIFI_STA);  
    
    bool scanHidden=(filtered)?false:true;

    WIFISCAN_LOG_DEBUG(("WiFi start scanning\r\n"));
    uint32_t start = millis();

    if ( maxAp > WIFISCAN_MAX_AP ) maxAp = WIFISCAN_MAX_AP;
    this->wifiFound = 0;
    while ( (millis() - start) < timeoutMs && this->wifiFound < maxAp ) {
      int n = WiFi.scanNetworks(false,scanHidden);
      for(int i=0; i < n; i++){ 
        if ( filtered && filtering(i) ) continue;
        this->addWiFi(WiFi.BSSID(i),WiFi.RSSI(i),false);
      }
    }
    WIFISCAN_LOG_DEBUG(("WiFi scanning duration %d ms, found %d WiFi\r\n",millis()-start,this->wifiFound));

    WiFi.mode(WIFI_OFF);
    WiFi.forceSleepBegin();
    delay(1);
}

/**
 * Indicate if the given index entry have to be filtered or not (true if it have to be filtered)
 * Filter conditions
 * - Multicast (byte0, bit 0) = 1 
 * - Locally administred ( byte 0, bit 1) = 1
 * - Public SSID not containg keyworks like ( android, phone, samsung, huawei ) for ermoving mobile hotspot
 * - Full of 00 
 * - Full of FF
 */
bool WifiScanClass::filtering(int index) {

  // Test for Multicast, Locally Administred and Full of FF
  uint8_t * mac = WiFi.BSSID(index);
  uint8_t firstMacByte = mac[0];
  if ( (firstMacByte & 0x3) != 0 ) return true;

  // Test for full of 0
  if ( firstMacByte == 0 ) {
    int i=0;
    while ( i < 6 ) {
      if ( mac[i] != 0 ) break;
      i++;
    }
    if ( i == 6 ) return true;
  }

  // Test Hidden
  if ( WiFi.isHidden(index) ) return true;

  // Test SSID 
  String ssid = WiFi.SSID(index);
  ssid.toLowerCase();
  for (int i=0 ; i < WIFISCAN_SSIDFILTERLEN ; i++) {
    if ( ssid.indexOf(ssidFiltered[i]) > -1 ) return true;
  }

  return false;
}

/**
 * Print in the log file the Wifi Found during the last scan
 */
void WifiScanClass::printWiFi() {
  WIFISCAN_LOG_ANY(("+-------- WiFi Found -----+\r\n"));
  WIFISCAN_LOG_ANY(("|        MAC      |  RSSI |\r\n"));
  //                 |00:00:00:00:00:00|  -125 |
  for ( int i = 0 ; i < this->wifiFound ; i++ ) {

    WIFISCAN_LOG_ANY(("|"));
    for ( int j = 0 ; j < 6 ; j++ ) {
       WIFISCAN_LOG_ANY(("%02X",this->wifi[i].mac[j]));
       if ( j < 5 ) WIFISCAN_LOG_ANY((":"));
    }
    WIFISCAN_LOG_ANY(("| "));
    WIFISCAN_LOG_ANY(("%4d |\r\n",this->wifi[i].rssi));
     
  }
  WIFISCAN_LOG_ANY(("+-------------------------+\r\n")); 
}


/**
 * Search in the WiFi list the two offering the best RSSI and copy the MAC
 * address into the given mac1 and mac2 buffer. each have to be a uint8_t[6]
 * buffer. Returns the number of Wifi information returned 0 / 1 / 2
 */
int WifiScanClass::getFirstAndSecondBestWiFi(uint8_t * mac1, uint8_t * mac2) {
  switch(this->wifiFound){
    case 0:
      return 0;
    case 1:
      for (int k=0; k< 6 ; k++) mac1[k]=this->wifi[0].mac[k];
      return 1;
    default:
      int best1=0; int8_t rss1=-128;
      int best2=0; int8_t rss2=-128;
      for (int i=0 ; i<this->wifiFound ; i++) {
        if ( this->wifi[i].rssi >= rss1 ) {
           best2 = best1;
           rss2 = rss1;
           best1 = i;  
           rss1 = this->wifi[i].rssi;    
        } else if ( this->wifi[i].rssi >= rss2 ) {
           best2 = i;
           rss2 = this->wifi[i].rssi;
        }
        for (int k=0; k< 6 ; k++) mac1[k]=this->wifi[best1].mac[k];
        for (int k=0; k< 6 ; k++) mac2[k]=this->wifi[best2].mac[k];
      }
      return 2;
  }  
}

/**
 * Add a Wifi entry in the table list if not already existing
 * Update the rssi when better if exists
 * Rssi is modified to fit -128 to +127 range
 * When unicastOnly is true, only the MAC type unicast are added
 *  unicast is indicated by higher byte lower bit is 0
 */
void WifiScanClass::addWiFi(uint8_t * _mac, int32_t _rssi, bool unicastOnly)
{
  // filter the multicast addresses
  if ( unicastOnly && (_mac[0] & 0x01) == 0x01 ) return;

  t_wifiAp * entry = this->searchForWiFi(_mac);
  if ( entry == NULL && this->wifiFound < WIFISCAN_MAX_AP ) {
    // create a new WiFi entry
    for ( int j = 0 ; j < 6 ; j++ ) { 
      this->wifi[this->wifiFound].mac[j] = _mac[j];
    }
    this->wifi[this->wifiFound].rssi = (int8_t)((_rssi < -128)?-128:_rssi);
    this->wifiFound++;

  } else {
    // update the Rssi if better
    if ( _rssi > entry->rssi ) {
        entry->rssi = (int8_t)((_rssi < -128)?-128:_rssi);
    }
  }
  
}

/**
 * Serach in the wifi list a corresponding entry with the same
 * MAC adress. Return this entry when found, NULL otherwise.
 */
t_wifiAp * WifiScanClass::searchForWiFi(uint8_t * _mac) {

  for ( int i = 0 ; i < this->wifiFound ; i++ ) {
     uint8_t c = 0;
     while ( c < 6 && this->wifi[i].mac[c] == _mac[c] ) c++;
     if ( c == 6 ) return &this->wifi[i];
  }
  return NULL;
  
}
 

