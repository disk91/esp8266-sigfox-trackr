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

 #include "wisol.h"
 extern "C" {
   #include "tool.h"
 }
 #include "config.h"
 #include "SoftwareSerial.h"
 
 WisolClass wisolService;
 SoftwareSerial swSer1(WISOL_RX_PIN, WISOL_TX_PIN, false, 64);       // RX, TX, Invert, Buffer Size

 /**
  * Init the communication with Wisol.
  * This must be called before any Wisol access
  */
 bool WisolClass::init() {
  if ( ! ready ) {
    WISOL_LOG_INFO(("Init Wisol\r\n"));
    swSer1.begin(9600);
    delay(10);
    flushRxLine();
    ready = true;
  }
  return true;
 }

/**
 * Reset the wisol chip
 */
bool WisolClass::reset() {
  sendLine("AT$P=0\r");
  delay(1000);
  flushRxLine();
  return true;
 }



/**
 * Verify id the Wisol is sleeping
 * Should not respond to AT message
 * Rq : It is not possible to know if the device is sleeping or
 * not as any communication will wake it up ... sound not normal
 * as definition of line break is a Low level LARGER than a normal char
 * but real life !
 */
 /*
bool WisolClass::isSleeping() {
  char buf[100];
  
  sendLine("AT\r\n");
  if ( readLine(buf,100,WISOL_WAIT_STD_TIME_MS,false) ) {
    //TTRACE(("%s\r\n",buf));
    return false;
  } else {
    return true;
  }
}
*/


/**
 * Switch the Wisol device to sleep mode 
 * Normal standby mode is 500uA vs 2uA in sleep mode
 * Wake up with a UART Break
 */
bool WisolClass::sleepMode() {
  char buf[100];

  WISOL_LOG_INFO(("Wisol - request sleeping\r\n"));
  sendLine("AT$P=1\r");   // /!\ Note = if you add a LF (\n) at end of this command the redive returns OK but don't switch to sleep mode
  if ( readLine(buf,100,WISOL_WAIT_STD_TIME_MS,false) ) {
    if ( strcmp(buf,"OK") == 0 ) {
      WISOL_LOG_WARN(("wisol sleep request applied\r\n"));
      return true;
    } else {
      WISOL_LOG_DEBUG(("wisol sleep request returned : %s\r\n",buf));
      return false;
    }
  } else {
    // If not responding, looks strange
    return false;
  }
}


/**
 * Wake up wisol by sending a break on UART-TX
 * Break is LOW on tx durring ??? 
 */
void WisolClass::wakeUp() {
  WISOL_LOG_INFO(("Wisol - waking up\r\n"));
  init();
  digitalWrite(WISOL_TX_PIN,LOW);
  delay(5);
  digitalWrite(WISOL_TX_PIN,HIGH);
  delay(20);
  flushRxLine();                         
}


/**
 * Send a message to Sigfox of the indicated len.
 * When withDownlink is true, a downlink response is expected. The response will be stored in the downlink 8bytes given buffer 
 * The status returns is the following
 * Uplink :
 *   WISOL_STATUS_SEND_KO => Fame not transmitted
 *   WISOL_STATUS_SEND_OK => Frame transmitted
 *   WISOL_STATUS_NO_DONWLINK => Frame transmitted, no downlink response
 *   WISOL_STATUS_DOWNLINK => Frame trasnmitted, downlink response received
 *   
 * Rq : Downlink feature are not yet implemented
 */
int WisolClass::sendRaw(uint8_t * frame, int len, bool withDownlink, uint8_t * downlink) {

  if ( len > 12 ) return WISOL_STATUS_SEND_KO;
 
  char msg[25];
  dsk_convertIntTab2Hex(msg,frame,len,true);
  if ( !withDownlink ) {
   char cmd[128];
   sprintf(cmd,"AT$SF=%s\r",msg); 
   WISOL_LOG_DEBUG(("Wisol is sending the following command : [%s]\r\n",cmd));
   sendLine(cmd);
   if ( readLine(cmd,128,WISOL_WAIT_UPLINK_MS,false) ) {
      if ( strcmp(cmd,"OK") == 0 ) {
        return WISOL_STATUS_SEND_OK;
      } else {
        WISOL_LOG_ERROR(("Wisol uplink returned an invalid response (%s)\r\n",cmd));
        return WISOL_STATUS_SEND_KO;
      }
   } else {
      WISOL_LOG_ERROR(("Wisol uplink did not return response\r\n"));
      return WISOL_STATUS_SEND_KO;
   }

  } else {
    WISOL_LOG_ERROR(("Wisol downlink support not yet implemented\r\n"));
  }
  
}

/**
 * Get the sigfox PAK stored in Wisol module. The string is stored
 * in the given buffer with a maxium of sz char (-1)
 * Return true when success
 */
bool WisolClass::getSigfoxPak(char * buf, int sz) {
  bool ret = false;
  sendLine("AT$I=11\r");
  if ( readLine(buf,sz,WISOL_WAIT_STD_TIME_MS,false) ) {
    if ( strlen(buf) == 16 && dsk_isHexString(buf,strlen(buf),true) ) {
      ret = true;
    } else {
      WISOL_LOG_WARN(("Invalid response (PAK) from Wisol\r\n"));
    }
  } else {
    WISOL_LOG_WARN(("No response (PAK) from Wisol\r\n"));
    ret = false;
  }
  
  return ret;
}

/**
 * Same as getSigfoxPak but make retry in case of communication error 
 */
bool WisolClass::getSigfoxPakWithRetry(char * buf, int sz,int retry) {
  bool ret;
  while (retry>0 && !(ret=getSigfoxPak(buf,sz)) ) { retry-- ; delay(10); }
  return ret;
}


/**
 * Get the sigfoxId from the Wisol device
 * Return 0 in case of error
 */
uint32_t WisolClass::getSigfoxId() {
  char buf[32];
  uint32_t ret;

  sendLine("AT$I=10\r");
  if ( readLine(buf,32,WISOL_WAIT_STD_TIME_MS,false) ) {
     if ( strlen(buf) == 8 && dsk_isHexString(buf,8,false) ){
       ret = dsk_convertHexChar8Int(buf);
     } else {
      WISOL_LOG_WARN(("Invalid response (ID) from Wisol\r\n"));
      ret = 0;
     }
  } else {
    WISOL_LOG_WARN(("No response (ID) from Wisol\r\n"));
    ret = 0;
  }

  return ret;
}

/**
 * Same as getSigfoxId but make retry in case of communication error 
 */
uint32_t WisolClass::getSigfoxIdWithRetry(int retry) {
  uint32_t ret;
  while (retry>0 && !(ret=getSigfoxId()) ) { retry-- ; delay(10); }
  return ret;
}


/**
 * Return the temperature as measured by the chip
 * The result is by 1/10 of Celcius degrees.
 * In case of error the result is -300 (INVALID_TEMPERATURE)
 */
int16_t WisolClass::getTemperature() {
  char buf[100];
  sendLine("AT$T?\r");
  if ( readLine(buf,100,WISOL_WAIT_STD_TIME_MS,false) ) {
    int16_t temp = -300;
    if ( strlen(buf) >= 4 ) {
      temp = dsk_convertDecChar4Int(buf);
    }
    WISOL_LOG_DEBUG(("Wisol Temperature : %s -- %d\r\n",buf,temp));
    return temp;
  } else {
    return WISOL_INVALID_TEMPERATURE;
  }
}

/**
 * Same as getTemperature but make retry in case of communication error 
 */
uint16_t WisolClass::getTemperatureWithRetry(int retry) {
  uint16_t ret;
  while (retry>0 && (ret=getTemperature()) == WISOL_INVALID_TEMPERATURE ) { retry-- ; delay(10); }
  return ret;
}


/**
 * Return the Wisol voltage in mV. In case of error 0 is returned
 */
uint16_t WisolClass::getVoltage() {
  char buf[100];
  sendLine("AT$V?\r");
  if ( readLine(buf,100,WISOL_WAIT_STD_TIME_MS,false) ) {
    uint16_t volt = 0;
    if ( strlen(buf) >= 4 ) {
      volt = dsk_convertDecChar4Int(buf);
    }
    WISOL_LOG_DEBUG(("Wisol Volt : %s -- %d\r\n",buf,volt));
    return volt;
  } else {
    return WISOL_INVALID_VOLTAGE;
  }
}
/**
 * Same as getTemperature but make retry in case of communication error 
 */
uint16_t WisolClass::getVoltageWithRetry(int retry) {
  uint16_t ret;
  while (retry>0 && (ret=getVoltage()) == WISOL_INVALID_VOLTAGE ) { retry-- ; delay(10); }
  return ret;
}


// ==========================================================================
// Internal functions

void WisolClass::flushRxLine() {
  while ( swSer1.available() ) swSer1.read(); 
}

void WisolClass::sendLine(const char * str) {
  init();
  int len = strlen(str);
  if ( len > 1 ) {
    for ( int i = 0 ; i < len-1 ; i++ ) {
      swSer1.print(str[i]);
      swSer1.flush();
      delay(100);
    }
    swSer1.print(str[len-1]);
  } else {
    swSer1.print(str);
  }
}

 /**
  * Read a line from the SerialLine
  * Blocking util read terminated or timeout in Ms
  * when withEol is true \r and \n are also copied to buffer
  * Check the content of the received line, when starting with "ERROR:" it returns false.
  * Return true when getting in time a string chain not an error
  */
 bool WisolClass::readLine(char * _buf,int sz, int maxMs, bool withEol) {

   char * buf =_buf;
   bool end = false;
   char c;
   while ( true ) {
     while ( !swSer1.available() && maxMs > 0 ) { delay(1); maxMs--; }
     if ( maxMs == 0 ) return false;
     while ( swSer1.available() > 0 && sz > 1) {
        c=swSer1.read();
        if ( withEol || (c != '\r' && c != '\n') ) {
          *buf=c;
          buf++;
          sz--;
        }
     }
     if ( c == '\n' ) {
        *buf='\0';
        //TTRACE2(("rTime : %d\r\n",maxMs ));
        if ( strncmp(_buf,"ERROR:",6) == 0 ) {
           WISOL_LOG_DEBUG(("Wisol serial err\r\n"));
           return false; 
        }
        return true;
     }
     if ( sz <= 1 ) return false;
   }
 }

