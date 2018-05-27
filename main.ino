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
 *  ESP8266 Main module
 * ----------------------------------------------------------------------
 * (c) Disk91.com - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */
#include <ESP8266WiFi.h>

#include "config.h"
#include "debug.h"
#include "logger.h"
#include "low_power.h"
#include "tracker.h"

int  bootTime,bootCycle;
bool debugMode = false;
bool debugModeLoop = false;
bool inCommandMode = false;

void setup() {
  bootTime = millis();
  
  // Enable WatchDog
  ESP.wdtEnable(32000);                                                                       // 32s watchdog
  Serial.begin(LOGGER_SERIAL_DEFAULT_SPEED);                                                  // needed for config load & lowpower trace on Serial

  // Disable Wifi
  WiFi.disconnect(); 
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
  
  // On start, check the STOP PIN, in zero => stop execution
  // This is protecting against programmation issue when running deep sleep continuously
  pinMode(D5,INPUT);
  if ( digitalRead(D5) == LOW ) {
    debugMode = true;
  }

  TTRACE1(("BootTime %d \r\n", bootTime));


}

void manageCommand() {
  char c;
  if ( Serial.available() ) {
     char c = Serial.read();
     if ( c == '!' ) {
        inCommandMode = true;
     } else if ( inCommandMode ) {
        if ( c == 'd' ) { TTRACE(("switch to debug Mode\r\n")); debugMode = true; }
        trackrService.processCommands(c);

        inCommandMode = false;
     }
  }
}


void loop() {

    // ----
    // For real this loop will be executed only one time after every deep sleep wake up
    uint32_t elapsed = ( debugModeLoop )? 0 : millis(); 
    
    if ( debugModeLoop || lowPowerService.wakeUp((uint8_t*)&trackrService.state, &trackrService.state.crc32, sizeof(trackrService.state)) ) {

      // This is a standard loop from a device wake up signal or after an internal wait loop
      // We execute all what we have to do on regular basis
    
      trackrService.execute(SCHEDULER_PERIOD_MS+elapsed);                              // Load the context from RTC memory & execute actions
    
    } else {

      // This is the first loop after powering ON the device
      // We can call boot method to execute all the first time settings 
      
      trackrService.boot(elapsed+5000);                                               // What have to be done just one time on startup from a reset
      // wait for potential debug mode switch for 5 seconds
      uint32_t start = millis();
      while ( (millis() - start) < 5000 ) {
        manageCommand();
      }
    }

    if ( ! debugMode ) {
      // In the normal mod the ESP8266 is going deep sleep
      // going deep sleep...
      lowPowerService.deepSleep( SCHEDULER_PERIOD_MS,(uint8_t*)&trackrService.state, &trackrService.state.crc32, sizeof(trackrService.state) );
   
    } else {
      // debug mode, no sleep, always run so we can listen for command on the
      // serial line
      uint32_t start = millis();
      while ( (millis() - start) < SCHEDULER_PERIOD_MS ) {
        manageCommand();
        delay(1);
        yield();
        ESP.wdtFeed();
      }
    }
    if ( debugMode ) debugModeLoop = true;
}
