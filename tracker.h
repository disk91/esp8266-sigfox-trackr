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
#ifndef TRACKR_H_
#define TRACKR_H_

#include <Arduino.h>
#include "config.h"

typedef struct s_state {
      uint64_t  totalMs;
      uint8_t tab[128];
       
      uint32_t  crc32;        // zone to store RTC crc32
} t_state;


class TrackrClass {
public:
  t_state state;
  
  bool init();
  void boot(uint32_t elapsedTime);
  void execute(uint32_t elapsedTime);

  void processCommands(char c);
  
protected:
  void printTime();

};

extern TrackrClass trackrService;





#endif
