/* ======================================================================
    This file is part of disk91_sigfox.

    disk91_sigfox is free software: you can redistribute it and/or modify
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
 *  WISOL / Sigfox module / management module
 * ----------------------------------------------------------------------
 * (c) Disk91 - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 * Requires : SoftwareSerial - https://github.com/plerup/espsoftwareserial
 *                             /!\ version > 3.4.1 is recommanded
 */


#ifndef WISOL_H_
#define WISOL_H_

#include <Arduino.h>
#include "logger.h"

#define WISOL_LOG_LEVEL 5                    // 5 - Debug | 4 - Info | 3 - Warn | 2 - Error | 1 - Any | 0 - None
#define WISOL_WAIT_STD_TIME_MS   50          // Wait time in MS for wisol responding on standard short operation like config access
#define WISOL_WAIT_UPLINK_MS   12000         // Wait time in MS for wisol responding on a frame transmition


#define WISOL_INVALID_TEMPERATURE     -300    
#define WISOL_INVALID_VOLTAGE         0    

#define WISOL_STATUS_SEND_KO      0
#define WISOL_STATUS_SEND_OK      1
#define WISOL_STATUS_NO_DONWLINK  2
#define WISOL_STATUS_DOWNLINK     3

class WisolClass {
public:
  bool reset();

  int sendRaw(uint8_t * frame, int len, bool withDownlink, uint8_t * downlink);
  
  uint32_t getSigfoxId();
  uint32_t getSigfoxIdWithRetry(int retry);
  bool getSigfoxPak(char * buf, int sz);
  bool getSigfoxPakWithRetry(char * buf, int sz, int retry);

  int16_t getTemperature();
  uint16_t getTemperatureWithRetry(int retry);
  uint16_t getVoltage();
  uint16_t getVoltageWithRetry(int retry);

  // Power Management
  void wakeUp();
  bool sleepMode(); 
  //bool isSleeping();
  
  
protected:
  bool init();
  bool readLine(char * buf,int sz, int maxMs,bool withEol);
  void sendLine(const char * str);
  void flushRxLine();
  bool ready = 0;
  
};

extern WisolClass wisolService;

// Logger wrapper
#if WISOL_LOG_LEVEL >= 5
#define WISOL_LOG_DEBUG(x) _log.debug x
#else
#define WISOL_LOG_DEBUG(x) 
#endif

#if WISOL_LOG_LEVEL >= 4
#define WISOL_LOG_INFO(x) _log.info x
#else
#define WISOL_LOG_INFO(x) 
#endif

#if WISOL_LOG_LEVEL >= 3
#define WISOL_LOG_WARN(x) _log.warn x
#else
#define WISOL_LOG_WARN(x) 
#endif

#if WISOL_LOG_LEVEL >= 2
#define WISOL_LOG_ERROR(x) _log.error x
#else
#define WISOL_LOG_ERROR(x) 
#endif

#if WISOL_LOG_LEVEL >= 1
#define WISOL_LOG_ANY(x) _log.any x
#else
#define WISOL_LOG_ANY(x) 
#endif

#endif
