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

#ifndef LOGGER_H_
#define LOGGER_H_

#include <Arduino.h>
#include <FS.h>
#include <SoftwareSerial.h>

#define LOGGER_SERIAL_DEFAULT_SPEED   74880
#define LOGGER_SERIAL1_DEFAULT_SPEED  115200
#define LOGGER_MAX_BUF_SZ             256    

#define LOGGER_CONFIG_FILE_MASK       0xF000
#define LOGGER_CONFIG_SSERIAL_MASK    0x0F00
#define LOGGER_CONFIG_SERIAL1_MASK    0x00F0
#define LOGGER_CONFIG_SERIAL_MASK     0x000F
#define LOGGER_CONFIG_DEBUG_LVL_MASK  0x8888
#define LOGGER_CONFIG_INFO_LVL_MASK   0x4444
#define LOGGER_CONFIG_WARN_LVL_MASK   0x2222
#define LOGGER_CONFIG_ERROR_LVL_MASK  0x1111

#define LOGGER_FILE_MAX_SIZE          200000    // 200k - Max log file size - after this size the log file is deleted

class LoggerClass {
public:
  bool init(uint16_t config);
  uint16_t close();
  void error(char *format, ...);
  void warn(char *format, ...);
  void info(char *format, ...);
  void debug(char *format, ...);
  void any(char *format, ...);

  void cat();
  void clean();
  
  
protected:
  bool ready;         // Initialization has been done
  bool logError;      // Error log level reported somewhere
  bool logWarn;       // Warn log level reported somewhere
  bool logInfo;       // Info log level reported somewhere
  bool logDebug;      // Debug log level reported somewhere
  bool onSerial;      // Some logs are reported on Serial Line
  bool onSerial1;     // Some logs are reported on Serial1 Line
  bool onSSerial;     // Some logs are reported on SoftwareSerial Line
  bool onFile;        // Some logs are reported on Falsh file
  uint16_t  logConf;  // Detailed log level

  File logFile;
  SoftwareSerial * SSerial = NULL;
  
  char fmtBuffer[LOGGER_MAX_BUF_SZ];   // buffer for log line formating before printing
  
};

extern LoggerClass _log;

#endif
