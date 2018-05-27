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

#ifndef LOWPOWER_H_
#define LOWPOWER_H_

class LowPowerClass {
public:
  bool wakeUp(uint8_t * context, uint32_t * crc32area, unsigned int sz);
  void deepSleep(uint32_t durationMs, uint8_t * context, uint32_t * crc32area, unsigned int sz);

};

extern LowPowerClass lowPowerService;
#endif
