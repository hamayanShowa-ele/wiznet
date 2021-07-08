/* ----------------------------------------
  echo utilities
  for STMicroelectronics SPL library

  THE SOURCE CODE OF THE FOLLOWING url WAS MODIFIED FOR STM32F.
  https://github.com/asukiaaa/AM2320_asukiaaa

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
  time server : ntp.nict.jp
---------------------------------------- */

#ifndef _ECHO_H_
#define _ECHO_H_

#include  <HardwareSerial.h>
#include  <strutil.h>
#include  <wiznet.h>

extern "C"
{
  #include  <stdio.h>
  #include  <time.h>
  #include  <mul_tsk.h>
  #include  <socket.h>
  #include  <byteOrder.h>
}

/* ----------------------------------------
    prototypes
---------------------------------------- */

/* ----------------------------------------
    defines
---------------------------------------- */
#define ECHO_PORT  7     //echo server port number

class TCP_ECHO : public WIZNET_SOCKET
{
private:
  void loopback();

public:
  TCP_ECHO() {}
  TCP_ECHO( uint8_t s, const uint8_t *ip, uint16_t dp = ECHO_PORT, uint16_t sp = 0 )
    { begin( s, ip, dp, sp );}
  TCP_ECHO( uint8_t s, uint16_t sp = ECHO_PORT )
    { begin( s, sp ); }
  ~TCP_ECHO() { end(); }

  int server();
  int client(
    const uint8_t *txBuf, int32_t txSize,
    uint8_t *rxBuf, int32_t rxSize );
};

#endif  /* _ECHO_H_ */
