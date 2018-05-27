/* ======================================================================
    This file is part of disk91_tools.

    disk91_tools is free software: you can redistribute it and/or modify
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
 *  ESP8266 Misc tooling functions
 * ----------------------------------------------------------------------
 * (c) Disk91 - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */

#ifndef TOOL_H_
#define TOOL_H_


uint32_t calculateCRC32(const uint8_t *data, size_t length);

// ------------------------------------------------------------------------
// Converters
char dsk_convertHalfInt2HexChar(uint8_t v,bool upper);
void dsk_convertInt2HexChar(uint8_t v, char * dest, bool upper);
void dsk_convertIntTab2Hex(char * dest, uint8_t * tab, int len, bool upper);
bool dsk_isHexChar(char c, bool upper);
bool dsk_isHexString(char * str,int n,bool upper);
uint8_t dsk_convertHexChar2HalfInt(char c);
uint8_t dsk_convertHexChar2Int(char * v);
uint32_t dsk_convertHexChar8Int(char * v);
uint16_t dsk_convertDecChar4Int(char * v);
void dsk_convertHexStr2IntTab(char * hexstr,uint8_t * tab, int len);
void dsk_macToString(char * str, uint8_t * mac);

#endif
