/* ----------------------------------------
  wiznet chip utilities
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

#ifndef __WIZ_NET_H__
#define __WIZ_NET_H__

#include  <derivertive.h>
#include  <strutil.h>

#if  (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200)
#include  <SPI.h>
#endif  /* (_WIZCHIP_ == 5500) */

extern "C"
{
  #include  <string.h>
  #include  <mul_tsk.h>
  #include  <wizchip_conf.h>
  #include  <socket.h>
  #include  <byteOrder.h>
}

#if  (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200)
extern "C"
{
  #if  (_WIZCHIP_ == 5500)
  #include  <w5500.h>
  #endif  /* (_WIZCHIP_ == 5500) */
  #if  (_WIZCHIP_ == 5100)
  #include  <w5100.h>
  #endif  /* (_WIZCHIP_ == 5500) */
}
#define  _CHIP_VERSION_           0x04
#endif  /* (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200) */


class WIZNET : public GPIO
{
private:
#if  (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200)
  SPI *spi;
  uint8_t csPin;
  void (*cs_select)( void );
  void (*cs_deselect)( void );
#elif  (_WIZCHIP_ == 5300)
#endif  /* (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200) */

  void setNetworkInformations( dhcp_mode dhcp = NETINFO_STATIC );

public:
  uint8_t _MAC[6];      // self hardware address
  uint8_t _IPVAL[4];    // self ip address
  uint8_t _GATEWAY[4];  // default gateway
  uint8_t _SUBNET[4];   // sub net mask
  uint8_t _DNS[4];      // primary dns server

  WIZNET();
  ~WIZNET();

  void clearNetworkInfo();
#if  (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200)
  void begin(
    SPI *wire, const uint8_t *memsize,
    void (*_s)(), void (*_d)(), dhcp_mode dhcp = NETINFO_STATIC );
#elif  (_WIZCHIP_ == 5300)
  void begin( const uint8_t *memsize, dhcp_mode dhcp = NETINFO_STATIC );
#endif  /* (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200) */

  void getMac( uint8_t *dest );
  void getIp( uint8_t *dest );
  void getGateway( uint8_t *dest );
  void getSubnet( uint8_t *dest );
  void getDns( uint8_t *dest );

  void setMac( const uint8_t *src );
  void setIp( const uint8_t *src );
  void setGateway( const uint8_t *src );
  void setSubnet( const uint8_t *src );
  void setDns( const uint8_t *src );

  void setMac( uint8_t m0, uint8_t m1, uint8_t m2, uint8_t m3, uint8_t m4, uint8_t m5 )
    { _MAC[0] = m0; _MAC[1] = m1; _MAC[2] = m2; _MAC[3] = m3; _MAC[4] = m4; _MAC[5] = m5; }
  void setIp( uint8_t i0, uint8_t i1, uint8_t i2, uint8_t i3 )
    { _IPVAL[0] = i0; _IPVAL[1] = i1; _IPVAL[2] = i2; _IPVAL[3] = i3; }
  void setGateway( uint8_t g0, uint8_t g1, uint8_t g2, uint8_t g3 )
    { _GATEWAY[0] = g0; _GATEWAY[1] = g1; _GATEWAY[2] = g2; _GATEWAY[3] = g3; }
  void setSubnet( uint8_t s0, uint8_t s1, uint8_t s2, uint8_t s3 )
    { _SUBNET[0] = s0; _SUBNET[1] = s1; _SUBNET[2] = s2; _SUBNET[3] = s3; }
  void setDns( uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3 )
    { _DNS[0] = d0; _DNS[1] = d1; _DNS[2] = d2; _DNS[3] = d3; }
};



class WIZNET_SOCKET // : public hoge
{
private:
  void ( *userFunction )();

public:
  uint8_t soc;
  uint8_t dip[4];
  uint16_t sport,dport;
  uint8_t destIP[4]; uint16_t destPort;

  WIZNET_SOCKET() {}
  ~WIZNET_SOCKET() {}

  void begin( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp = 0 );
  void begin( uint8_t s, uint16_t sp );
  void end();

  int8_t Open( uint8_t mode, uint8_t flg = 0, SYSTIM tmout = 1000UL );
  int8_t Close();
  int8_t Listen();
  int8_t Connect();
  int8_t Disconnect();
  int32_t Send( const uint8_t *data, int32_t len );
  int32_t Recv( uint8_t *data, int32_t len, SYSTIM tmout = 1000UL );
  int32_t Sendto( const uint8_t *data, int32_t len );
  int32_t Recvfrom( uint8_t *data, int32_t len, SYSTIM tmout = 1000UL );
  void shutDown();

  int8_t Getsockopt( sockopt_type sotype, void* arg );
  int8_t CtlSocket( ctlsock_type cstype, void* arg );

#if 0
  void regUserFuncution( void(*_f)() ) { userFunction = _f; }
  int  Server( void(*_f)() );
#endif
};

#endif  /* __WIZ_NET_H__ */
