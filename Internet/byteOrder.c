/* ----------------------------------------
 network byte order exchange.
  for STMicroelectronics SPL library

  Copyright (c) 2020 hamayan (hamayan@showa-ele.jp).
  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

  Created 2020 by hamayan (hamayan@showa-ele.jp)
---------------------------------------- */
#include  "byteOrder.h"

/* ----------------------------------------
    prototypes 
---------------------------------------- */

/* ----------------------------------------
    instances or global variables
---------------------------------------- */

/* ----------------------------------------
    swap 16bit
---------------------------------------- */
uint16_t swap16( uint16_t w )
{
  uint16_t l = (w >> 8) & 0x00FF;
  uint16_t u = (w << 8) & 0xFF00;
  return u | l;
}

/* ----------------------------------------
    swap 32bit
---------------------------------------- */
uint32_t swap32( uint32_t dw )
{
  uint32_t ll = (dw >> 24) & 0x000000FF;
  uint32_t lu = (dw >> 8)  & 0x0000FF00;
  uint32_t ul = (dw << 8)  & 0x00FF0000;
  uint32_t uu = (dw << 24) & 0xFF000000;
  return uu | ul | lu | ll;
}

