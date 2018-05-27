/* ======================================================================
    This file is part of disk91_lowpower.

    disk91_lowpower is free software: you can redistribute it and/or modify
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
 *  ESP8266 Low power management module
 * ----------------------------------------------------------------------
 * (c) Disk91 - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */

extern "C" {
#include <user_interface.h>
#include "tool.h"
}
#include <esp.h>
#include "config.h"
#include "debug.h"
#include "low_power.h"




LowPowerClass lowPowerService;


/**
 * When the device is restarting this function is called at first.
 * In case of reset startup the function just return false.
 * In case of restart after deep sleep the function restore context and return true
 * A byte buffer where to restaure the data is provided as context.
 * In this byte buffer, a specific area contains the location for crc32 to ensure a correct restore.
 */
bool LowPowerClass::wakeUp(uint8_t * context, uint32_t * crc32area, unsigned int sz) {

  if ( sz > RTC_MAX_SZ ) {
    TTRACE(("** Invalid context size !\r\n"));
    while(true);
  }

  rst_info * rstInfo = ESP.getResetInfoPtr(); 
  if ( rstInfo->reason == REASON_DEEP_SLEEP_AWAKE ) {
    TTRACE1(("Wake Up from deep-sleep\r\n"));
    // Restoring context from the RTC Memory    
    if ( ESP.rtcUserMemoryRead(0, (uint32_t*) context, sz) ) {
      uint32_t crc = *crc32area;
      *crc32area = 0;
      if ( crc != calculateCRC32((uint8_t*) context, sz) ) {
        TTRACE(("CRC32 Error during RTC Context restoration\r\n"));
        ESP.reset();
      }    
    } else {
      TTRACE(("Error during RTC Context restoration\r\n"));
      ESP.reset();
    }
    return true;
  } else {
    TTRACE1(("Wake-up : Reset detected \r\n"));
    return false;
  }
  
}

/**
 * The device is entering in deepSleep Mode for the given time in Ms. After this time
 * the GPIO16 (D0 on D1-mini) will go low. Connected to RST pin it will restart the device
 * During restart the cause will indicate what to do on restart.
 * Context of execution is stored in RTC memory and restore. Max size is 512 Bytes
 */
void LowPowerClass::deepSleep(uint32_t durationMs, uint8_t * context, uint32_t * crc32area, unsigned int sz) {
  *crc32area = 0;
  *crc32area = calculateCRC32((uint8_t*) context, sz);
  if ( ! ESP.rtcUserMemoryWrite(0, (uint32_t*) context, sz) ) {
     TTRACE(("Error when writting RTC Memory\r\n"));
  }
  ESP.deepSleep( durationMs * 1000L, WAKE_RF_DISABLED );
}

