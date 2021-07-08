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
---------------------------------------- */
#include "echo.h"

/* ----------------------------------------
    prototypes 
---------------------------------------- */

/* ----------------------------------------
    instances or global variables
---------------------------------------- */

/* ----------------------------------------
    constructor destructor
---------------------------------------- */

/* ----------------------------------------
    server
---------------------------------------- */
int TCP_ECHO::server()
{
  uint8_t status;
  int ret = SOCK_OK;

  Getsockopt( SO_STATUS, &status );
  ret = status;
  switch ( status )
  {
    case SOCK_CLOSED :  // CLOSED 0x00
      Close();
      Open( Sn_MR_TCP );
      break;
    case SOCK_INIT :  // The SOCKET opened with TCP mode 0x13
      Listen();
      break;
    case SOCK_ESTABLISHED :  // ESTABLISHED? 0x17
      loopback();
      break;
    case SOCK_CLOSE_WAIT :  // PASSIVE CLOSED 0x1C
      Disconnect();
      break;
    case SOCK_LISTEN :  // 0x14
    case SOCK_SYNSENT :  // 0x15
    case SOCK_SYNRECV :  // 0x16
      break;
    case SOCK_FIN_WAIT :  // 0x18
    case SOCK_CLOSING :  // 0x1A
    case SOCK_TIME_WAIT :  // 0x1B
    case SOCK_LAST_ACK :  // 0x1D
      break;
    case 0x10 :   // what is this ? 0x10
    case 0x11 :   // what is this ? 0x11
      break;
    default :
      Disconnect();
      ret = SOCK_FATAL;
      break;
  }

  return ret;
}

/* ----------------------------------------
    client
---------------------------------------- */
int TCP_ECHO::client(
  const uint8_t *txBuf, int32_t txSize,
  uint8_t *rxBuf, int32_t rxSize )
{
#if 1
  /* transmit process  */
  int32_t ret = Send( txBuf, txSize );
  if( ret <= 0 ) return ret;

  /* recieve process  */
  ret = Recv( rxBuf, rxSize, 1000UL );

  return ret;
#else
  /* Subtle code... */
  uint8_t status;
  int32_t ret = ECHO_OK;

  Getsockopt( SO_STATUS, &status );
  ret = status;
  switch ( status )
  {
    case SOCK_CLOSED :  // CLOSED 0x00
      Close();
      Open( Sn_MR_TCP );
      break;
    case SOCK_INIT :  // The SOCKET opened with TCP mode 0x13
      Connect();
      break;
    case SOCK_ESTABLISHED :  // ESTABLISHED? 0x17
      /* transmit process  */
      ret = Send( txBuf, txSize );
      if( ret <= 0 ) return ret;
      /* recieve process  */
      ret = Recv( rxBuf, rxSize, 1000UL );
      break;
    case SOCK_CLOSE_WAIT :  // PASSIVE CLOSED 0x1C
      Disconnect();
      break;
    case SOCK_FIN_WAIT :  // 0x18
    case SOCK_CLOSING :  // 0x1A
    case SOCK_TIME_WAIT :  // 0x1B
    case SOCK_LAST_ACK :  // 0x1D
      break;
    case 0x10 :   // what is this ? 0x10
    case 0x11 :   // what is this ? 0x11
      break;
    case SOCK_LISTEN :  // 0x14
    case SOCK_SYNSENT :  // 0x15
    case SOCK_SYNRECV :  // 0x16
    default :
      Disconnect();
      ret = ECHO_ERROR;
      break;
  }
  return ret;
#endif
}

/* ----------------------------------------
    loopback
---------------------------------------- */
void TCP_ECHO::loopback()
{
  uint16_t rcvSize;
  Getsockopt( SO_RECVBUF, &rcvSize );  // The rcvSize will be 2 bytes more than the actual received size.
  uint8_t *buf = new uint8_t[ rcvSize ];
  int32_t result = Recv( buf, rcvSize, 0UL );
  if( result > 0 )
  {
    Send( (const uint8_t *)buf, (int)result );
  }
  delete [] buf;
}
