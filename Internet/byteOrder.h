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
#ifndef  __BYTE_ORDER_H__
#define  __BYTE_ORDER_H__

#if  defined( STM32F10X_HD ) || defined( STM32F10X_MD )
#include  <stm32f10x.h>
#elif  defined( STM32F4XX )
#include  <stm32f4xx.h>
#endif  /* STM32F10X_HD */

/* ----------------------------------------
    prototypes
---------------------------------------- */
uint16_t swap16( uint16_t w );
uint32_t swap32( uint32_t dw );

/* ----------------------------------------
    defines
---------------------------------------- */
#define  htonl(a)  swap32(a)
#define  htons(a)  swap16(a)
#define  ntohl(a)  swap32(a)
#define  ntohs(a)  swap16(a)

/* ----------------------------------------
    instances or global variables
---------------------------------------- */



#endif  /* __BYTE_ORDER_H__ */

