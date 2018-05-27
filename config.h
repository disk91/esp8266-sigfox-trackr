/* ======================================================================
    This file is part of disk91_config.

    disk91_config is free software: you can redistribute it and/or modify
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
 *  ESP8266 config module
 * ----------------------------------------------------------------------
 * (c) Disk91 - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <Arduino.h>

// -------------------------------------------------
// Version
#define HARDWARE_VERSION  0x01
#define FIRMWARE_VERSION  0x01

// -------------------------------------------------
// Scheduling
//#define SCHEDULER_PERIOD_MS (30*1000)            
#define SCHEDULER_PERIOD_MS (15*60*1000)            // 15 minutes scan and transmission


// -------------------------------------------------
// Trace & Debug
#define DEBUG 3
#define CONFIG_LOGGEUR  0xF0FF      // all level in file and Serial

// -------------------------------------------------
// HARDWARE PINOUT
#define WISOL_RX_PIN  D6
#define WISOL_TX_PIN  D7
#define DEBUG_SOFTSERIAL_RX_PIN 0
#define DEBUG_SOFTSERIAL_TX_PIN 0


// -------------------------------------------------
// HARDWARE ESP DEFINES
#define EPROM_MAX_SZ  512
#define RTC_MAX_SZ    512
#define EEPROM_MAGIC  0xA5FC



typedef struct s_config {
      // mandatory fields for config management
        uint16_t  magic;
        uint16_t  size;
        uint32_t  crc32;

      // here are the project specific settings
        uint8_t   firmwareVersion;
        uint16_t  logConfig;          // see logger.cpp to get the format
        uint32_t  sigfoxId;
      
} t_config;

class ConfigClass {
public:
     t_config config;

     bool init(bool forceReset);
     bool defaultConfig();
     bool loadConfig();
     void storeConfig();
     void printConfig();
     
protected:
    void setDefaultConfig();

 
};
extern ConfigClass configService;


#endif
