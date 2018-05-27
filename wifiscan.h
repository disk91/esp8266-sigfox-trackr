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

#ifndef WIFISCANNER_H_
#define WIFISCANNER_H_

#include <Arduino.h>
#include "logger.h"

#define WIFISCAN_LOG_LEVEL   5                    // 5 - Debug | 4 - Info | 3 - Warn | 2 - Error | 1 - Any | 0 - None
#define WIFISCAN_MAX_AP     32

typedef struct s_wifiAp {
    uint8_t   mac[6];
    int8_t    rssi;      
} t_wifiAp;


class WifiScanClass {
public:
  void startScan(uint32_t timeoutMs, uint8_t maxAp, boolean filtered);
  void printWiFi();
  int  getFirstAndSecondBestWiFi(uint8_t * mac1, uint8_t * mac2);
  
protected:
  uint8_t    wifiFound;
  t_wifiAp   wifi[WIFISCAN_MAX_AP];

  void addWiFi(uint8_t * _mac, int32_t _rssi, bool unicastOnly);
  bool filtering(int index);
  t_wifiAp * searchForWiFi(uint8_t * _mac);
};

extern WifiScanClass wifiscanService;

// Logger wrapper
#if WIFISCAN_LOG_LEVEL >= 5
#define WIFISCAN_LOG_DEBUG(x) _log.debug x
#else
#define WIFISCAN_LOG_DEBUG(x) 
#endif

#if WIFISCAN_LOG_LEVEL >= 4
#define WIFISCAN_LOG_INFO(x) _log.info x
#else
#define WIFISCAN_LOG_INFO(x) 
#endif

#if WIFISCAN_LOG_LEVEL >= 3
#define WIFISCAN_LOG_WARN(x) _log.warn x
#else
#define WIFISCAN_LOG_WARN(x) 
#endif

#if WIFISCAN_LOG_LEVEL >= 2
#define WIFISCAN_LOG_ERROR(x) _log.error x
#else
#define WIFISCAN_LOG_ERROR(x) 
#endif

#if WIFISCAN_LOG_LEVEL >= 1
#define WIFISCAN_LOG_ANY(x) _log.any x
#else
#define WIFISCAN_LOG_ANY(x) 
#endif

#endif
