/* ======================================================================
    This file is part of disk91_tracker.

    disk91_tracker is free software: you can redistribute it and/or modify
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
 *  Tracker main module
 * ----------------------------------------------------------------------
 * (c) Disk91.com - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */
#include "tracker.h"
#include "config.h"
#include "debug.h"
#include "wisol.h"
#include "wifiscan.h"
#include "logger.h"
#include "wisol.h"
 extern "C" {
   #include "tool.h"
 }


TrackrClass trackrService;



/**
 * Function call on every boot to diplay informations, run init ...
 * elapsedTime = time elapsed in Ms since power on.
 */
void TrackrClass::boot(uint32_t elapsedTime) {
    char buf[128];
    uint32_t start = millis();

    // Wisol Hardware reset
    wisolService.wakeUp();
    wisolService.reset();

    // Init software components
    configService.init(true);                         // Load the configuration from flash or create it - @TODO : remove forcReset with false
    _log.init(configService.config.logConfig);        // init logging engine
    this->init();

    // Boot messages
    _log.any("*** boot - TrackR - version %02X \r\n",FIRMWARE_VERSION);
    _log.debug("Sdk version: %s\n", ESP.getSdkVersion()); 
    _log.debug("Core Version: %s\n", ESP.getCoreVersion().c_str());  
    _log.debug("Boot Version: %u\n", ESP.getBootVersion());  
    _log.debug("Boot Mode: %u\n", ESP.getBootMode());  
    _log.debug("CPU Frequency: %u MHz\n", ESP.getCpuFreqMHz());
    _log.debug("Flash Size: %u \n", ESP.getFlashChipSize());



    // Terminate boot
    wisolService.sleepMode();  
    delay(2000);
    _log.close();
    state.totalMs = elapsedTime + (millis() - start);
}

/**
 * This function is called on every wake-up and manage the other one
 * The context has already been restored in Main
 * Elapsed Time = time in Ms elapsed since Last call
 */
void TrackrClass::execute(uint32_t elapsedTime) {
    uint32_t start = millis();
    uint8_t  mac1[6];
    uint8_t  mac2[6];

    // Reinit hardware - reload config
    configService.init(false);                        // load the configuratin from flash
    _log.init(configService.config.logConfig);        // init logging engine

    // What we want to do on every wakeup
    this->printTime();

    // Scan for Wifi
    wifiscanService.startScan(6000,4,true);
    if ( wifiscanService.getFirstAndSecondBestWiFi(mac1, mac2) == 2 ) {
      char macStr[20];
      dsk_macToString(macStr,mac1);
      _log.info("1. %s\r\n",macStr);
      dsk_macToString(macStr,mac2);
      _log.info("2. %s\r\n",macStr);   

      // Prepare the frame !
      uint8_t msg[12];
      for ( int i = 0 ; i < 6 ; i++ ) {
        msg[i]=mac1[i];
        msg[i+6]=mac2[i];
      }
      wisolService.wakeUp();
      wisolService.sendRaw(msg,12,false,NULL);
      wisolService.sleepMode();
    }

    // Preparre to sleep
    _log.close();
    state.totalMs += elapsedTime + (millis() - start);
}


/**
 * Init the device state after a cold restart.
 */
bool TrackrClass::init() {
  _log.info("State Init\r\n");
  state.totalMs = 0;
  
}

/**
 * Update some timer on every call
 */
void TrackrClass::printTime() {
  int hour = state.totalMs / (3600*1000);
  int min  = (state.totalMs - (hour*(3600*1000))) / (60*1000); 
  int sec  = (state.totalMs - (hour*(3600*1000)) - (min*(60*1000))) / 1000;
  int ms   = (state.totalMs % 1000);
  _log.info("Time is : %d:%02d:%02d.%03d\n",hour,min,sec,ms);
}


void TrackrClass::processCommands(char c) {
  if ( c == '?' ) { _log.any("(c) 2018 Disk91.com\r\n"); }
  if ( c == 'l' ) { _log.cat(); }
  if ( c == 'C' ) { _log.any("Clean log file\n");_log.clean(); }
  if ( c == 'P' ) { char buf[64]; wisolService.wakeUp(); wisolService.getSigfoxPakWithRetry(buf,64,3); _log.any("Sigfox PAK : %s\n",buf); wisolService.sleepMode(); }
  if ( c == 'c' ) { configService.printConfig(); }

}

