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

#include "config.h"
extern "C" {
#include "tool.h"
}
#include "debug.h"
#include "wisol.h"
#include <EEPROM.h>

ConfigClass configService;

/**
 * Set the config structure with the factory default information
 */
void ConfigClass::setDefaultConfig() {
  // Mandtory init
  config.magic = EEPROM_MAGIC;
  config.size = sizeof(t_config);
  config.crc32 = 0x00;
  config.firmwareVersion = FIRMWARE_VERSION;

  // Project specific configuration
  config.sigfoxId = wisolService.getSigfoxIdWithRetry(3);
  config.logConfig = CONFIG_LOGGEUR;

  // -- end of project specific code
  config.crc32 = calculateCRC32((uint8_t*) &config, sizeof(t_config));
}


void ConfigClass::printConfig() {
  TTRACE(("-------- Config --------\r\n"));
  TTRACE((" Build %s %s\r\n",__DATE__,__TIME__));
  TTRACE((" Magic : %04X\r\n",config.magic));
  TTRACE((" HW version : %02X\r\n",HARDWARE_VERSION));
  TTRACE((" FW version : %02X\r\n",config.firmwareVersion));

  // Project specific configuration
  TTRACE((" SigfoxId : %08X\r\n",config.sigfoxId));
  TTRACE((" logConfig : %04X\r\n",config.logConfig));

}


/**
 * Force Init the device config 
 * return true if the default config has been flashed. 
 * Rq : No trace - at this point the trace configuration is not activated.
 */
bool ConfigClass::init(bool forceReset) {
  // Try to load the config from EEPROM
  if ( ! loadConfig() || forceReset ) {
    // Flash the default configuration
    TTRACE1(("Flash the default configuration\r\n"));
    setDefaultConfig();
    storeConfig(); 
    return true;   
  }
  return false;
}

/**
 * Store the config struture into the EEPROM
 */
void ConfigClass::storeConfig() {
   EEPROM.begin(EPROM_MAX_SZ);
   EEPROM.put(0,config);
   EEPROM.commit();
   EEPROM.end();
}


bool ConfigClass::loadConfig() {
  EEPROM.begin(EPROM_MAX_SZ);
  EEPROM.get(0,config);
  EEPROM.end();
    
  if ( config.magic == EEPROM_MAGIC ) {
    if ( config.size == sizeof(t_config) ) {
      uint32_t crc32 = config.crc32;
      config.crc32 = 0;
      if ( calculateCRC32((uint8_t*) &config, sizeof(t_config)) == crc32 ) {
        if ( config.firmwareVersion == FIRMWARE_VERSION) {
          // Load sucess
          return true;
        } else {
          TTRACE1(("Config Load Error - FW version\r\n")); 
        }
      } else {
        TTRACE1(("Config Load Error - CRC\r\n"));
      }
    } else {
      TTRACE1(("Config Load Error - Size\r\n"));      
    }
  } else {
    TTRACE1(("Config Load Error - Magic\r\n"));      
  }
  return false;
}


