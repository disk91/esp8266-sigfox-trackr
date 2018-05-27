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
 *  Debug settings
 * ----------------------------------------------------------------------
 * (c) Disk91 - 2018
 * Author : Paul Pinault aka disk91.com
 * ----------------------------------------------------------------------
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <config.h>

#ifdef DEBUG
#define TTRACE(x)   Serial.printf x
#define DTRACE(x)   Serial.printf(x)
#else
#define TTRACE(x)
#define DTRACE(x)
#endif

#if DEBUG > 1
#define TTRACE1(x)  Serial.printf x
#else
#define TTRACE1(x)
#endif

#if DEBUG > 2
#define TTRACE2(x)  Serial.printf x
#else
#define TTRACE2(x)
#endif

#if DEBUG > 3
#define TTRACE3(x)  Serial.printf x
#else
#define TTRACE3(x)
#endif

#define FLUSH(x) { Serial.flush(); delay(2); }

#define STOP_PIN  D5


#endif
