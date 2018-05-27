/* ======================================================================
    This file is part of disk91_logger.

    disk91_logger is free software: you can redistribute it and/or modify
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
 *  LOGGER / ESP8266 / logging module
 * ----------------------------------------------------------------------
 * (c) Disk91 - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */


#include "logger.h"
#include "config.h"

LoggerClass _log;

 /**
  * Init the logger structure from a given configuration
  * The configuration is a 32bits field for each of the 
  * 4 possible logging option with the 4 level of trace.
  * This configuration is optimize to be stored in the application
  * configuration and context to be easily saved & restore when 
  * going deep sleep.
  * Format is:
  *  +-------------------------------------------------------------------+
  *  | FILE output | SoftSerial output | Serial1 output | Serial output  |
  *  +-------------------------------------------------------------------+
  *  |   D I W E   |      D I W E      |     D I W E    |    D I W E     |
  *  +-------------------------------------------------------------------+
  *  D : DEBUG Level - activated when 1 / Mask when 0
  *  I : INFO Level - activated when 1 / Mask when 0
  *  W : WARN Level - activated when 1 / Mask when 0
  *  E : ERROR Level - activated when 1 / Mask when 0
  */
bool LoggerClass::init(uint16_t config) {

  this->logError = ( config & LOGGER_CONFIG_ERROR_LVL_MASK  ); 
  this->logWarn  = ( config & LOGGER_CONFIG_WARN_LVL_MASK   ); 
  this->logInfo  = ( config & LOGGER_CONFIG_INFO_LVL_MASK   ); 
  this->logDebug = ( config & LOGGER_CONFIG_DEBUG_LVL_MASK  ); 
  this->onSerial =  ( config & LOGGER_CONFIG_SERIAL_MASK    );
  this->onSerial1 = ( config & LOGGER_CONFIG_SERIAL1_MASK   );
  this->onSSerial = ( config & LOGGER_CONFIG_SSERIAL_MASK   );
  this->onFile =    ( config & LOGGER_CONFIG_FILE_MASK      );

  // Init the loggers   
  if (this->onSerial) {
    if ( !Serial ) {
       Serial.begin(LOGGER_SERIAL_DEFAULT_SPEED);
    }
    if ( Serial.baudRate() != LOGGER_SERIAL_DEFAULT_SPEED) {
       Serial.begin(LOGGER_SERIAL_DEFAULT_SPEED);      
    }
  }
    
  if (this->onSerial1) {
    Serial1.begin(LOGGER_SERIAL1_DEFAULT_SPEED);
  }

  if (this->onSSerial) {
    if ( SSerial == NULL ) SSerial = new SoftwareSerial(DEBUG_SOFTSERIAL_RX_PIN, DEBUG_SOFTSERIAL_TX_PIN, false, 256);       // RX, TX, Invert, Buffer Size
    SSerial->begin(9600);
  }

  if (this->onFile) {
    if ( !SPIFFS.begin() || !SPIFFS.exists("/formatComplete.txt") ) {
      // format fs
      SPIFFS.format();
      File f = SPIFFS.open("/formatComplete.txt", "w");
      f.close();
    }
    
    this->logFile = SPIFFS.open("/log.txt", "a");
    if ( this->logFile ) {
      if ( this->logFile.size() >= LOGGER_FILE_MAX_SIZE ) {
        this->logFile.close();
        SPIFFS.remove("/log.txt");
        this->logFile = SPIFFS.open("/log.txt", "a");
      }
    }
    if ( !this->logFile ) {    
      // problem, disable file logging
      this->onFile = false;
    }
  }
  this->logConf = config;
  this->ready = true;
  return true; 
}

/**
 * Terminate the logging. This should be called before going deep-sleep to flush
 * Current log processing.
 */
uint16_t LoggerClass::close() {
  if ( this->onFile) {
    this->logFile.close();
    SPIFFS.end();
  }
  if ( this->onSerial) {
    Serial.flush();
  }
  if ( this->onSerial1) {
    Serial1.flush();
  }
  if ( this->onSSerial) {
    SSerial->flush();
  }
  this->ready=false;
  return this->logConf;
}

/**
 * Print the log file over the serial line. As this is usually called during the
 * sleeping loop the SPIFF is supposed to be closed. The function try to manage this and 
 * restore the initial state.
 */
void LoggerClass::cat() {
  if ( this->ready && this->onFile) {
    this->logFile.close();
  }
  if ( !this->ready ) {
    SPIFFS.begin();
  }
  this->logFile = SPIFFS.open("/log.txt", "r");
  if (this->logFile) {
    Serial.printf("====== Read Log File (%db)=======\n",this->logFile.size());
    while ( this->logFile.available() ) {
       Serial.print((char)this->logFile.read());
    }
    Serial.println("====== end of Log File =======\n");    
    this->logFile.close();
  }
  if ( this->ready ) {        
    this->logFile = SPIFFS.open("/log.txt", "a");
  } else {
    SPIFFS.end();
  }
}

/**
 * Purge the log file
 * sleeping loop the SPIFF is supposed to be closed. The function try to manage this and 
 * restore the initial state.
 */
void LoggerClass::clean() {
  if ( this->ready && this->onFile) {
    this->logFile.close();
  }
  if ( !this->ready ) {
    SPIFFS.begin();
  }
  SPIFFS.remove("/log.txt");
  if ( this->ready ) {        
    this->logFile = SPIFFS.open("/log.txt", "a");
  } else {
    this->logFile.close();
    SPIFFS.end();
  }
}

/**
 * Log an error according to the configuration on the different
 * possible logger
 */
void LoggerClass::error(char *format, ...) {
  va_list args;
  if ( this->logError && this->ready ) {    
    va_start(args,format);
    vsnprintf(fmtBuffer,LOGGER_MAX_BUF_SZ,format,args);
    va_end(args);

    if ( this->logConf & LOGGER_CONFIG_SERIAL_MASK & LOGGER_CONFIG_ERROR_LVL_MASK ) {
      Serial.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SERIAL1_MASK & LOGGER_CONFIG_ERROR_LVL_MASK ) {
      Serial1.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SSERIAL_MASK & LOGGER_CONFIG_ERROR_LVL_MASK ) {
      SSerial->print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_FILE_MASK & LOGGER_CONFIG_ERROR_LVL_MASK ) {
      this->logFile.printf("%lu [error] ",millis());
      this->logFile.print(fmtBuffer);
    }
  
  }
}

/**
 * Log a warning according to the configuration on the different
 * possible logger
 */
void LoggerClass::warn(char *format, ...) {
  va_list args;
  if ( this->logWarn  && this->ready ) {    
    va_start(args,format);
    vsnprintf(fmtBuffer,LOGGER_MAX_BUF_SZ,format,args);
    va_end(args);

    if ( this->logConf & LOGGER_CONFIG_SERIAL_MASK & LOGGER_CONFIG_WARN_LVL_MASK ) {
      Serial.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SERIAL1_MASK & LOGGER_CONFIG_WARN_LVL_MASK ) {
      Serial1.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SSERIAL_MASK & LOGGER_CONFIG_WARN_LVL_MASK ) {
      SSerial->print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_FILE_MASK & LOGGER_CONFIG_WARN_LVL_MASK ) {
      this->logFile.printf("%lu [warn ] ",millis());
      this->logFile.print(fmtBuffer);
    }
  
  }
}


/**
 * Log a info according to the configuration on the different
 * possible logger
 */
void LoggerClass::info(char *format, ...) {
  va_list args;
  if ( this->logInfo  && this->ready ) {    
    va_start(args,format);
    vsnprintf(fmtBuffer,LOGGER_MAX_BUF_SZ,format,args);
    va_end(args);

    if ( this->logConf & LOGGER_CONFIG_SERIAL_MASK & LOGGER_CONFIG_INFO_LVL_MASK ) {
      Serial.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SERIAL1_MASK & LOGGER_CONFIG_INFO_LVL_MASK ) {
      Serial1.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SSERIAL_MASK & LOGGER_CONFIG_INFO_LVL_MASK ) {
      SSerial->print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_FILE_MASK & LOGGER_CONFIG_INFO_LVL_MASK ) {
      this->logFile.printf("%lu [info ] ",millis());
      this->logFile.print(fmtBuffer);
    }
  
  }
}

/**
 * Log a debug according to the configuration on the different
 * possible logger
 */
void LoggerClass::debug(char *format, ...) {
  va_list args;
  if ( this->logDebug  && this->ready ) {    
    va_start(args,format);
    vsnprintf(fmtBuffer,LOGGER_MAX_BUF_SZ,format,args);
    va_end(args);

    if ( this->logConf & LOGGER_CONFIG_SERIAL_MASK & LOGGER_CONFIG_DEBUG_LVL_MASK ) {
      Serial.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SERIAL1_MASK & LOGGER_CONFIG_DEBUG_LVL_MASK ) {
      Serial1.print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_SSERIAL_MASK & LOGGER_CONFIG_DEBUG_LVL_MASK ) {
      SSerial->print(fmtBuffer);
    }

    if ( this->logConf & LOGGER_CONFIG_FILE_MASK & LOGGER_CONFIG_DEBUG_LVL_MASK ) {
      this->logFile.printf("%lu [debug] ",millis());
      this->logFile.print(fmtBuffer);
    }
  
  }
}

/**
 * Log a debug according to the configuration on the different
 * possible logger
 */
void LoggerClass::any(char *format, ...) {
  va_list args;

  va_start(args,format);
  vsnprintf(fmtBuffer,LOGGER_MAX_BUF_SZ,format,args);
  va_end(args);

  if ( this->logConf & LOGGER_CONFIG_SERIAL_MASK ) {
    Serial.print(fmtBuffer);
  }

  if ( this->logConf & LOGGER_CONFIG_SERIAL1_MASK ) {
      Serial1.print(fmtBuffer);
  }

  if ( this->logConf & LOGGER_CONFIG_SSERIAL_MASK ) {
      SSerial->print(fmtBuffer);
  }

  if ( this->logConf & LOGGER_CONFIG_FILE_MASK ) {
      this->logFile.printf("%lu [any  ] ",millis());
      this->logFile.print(fmtBuffer);
  }
}

