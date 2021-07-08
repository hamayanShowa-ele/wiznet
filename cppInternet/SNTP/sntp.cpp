/* ----------------------------------------
  ntp utilities
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
#include "sntp.h"

/* ----------------------------------------
    prototypes 
---------------------------------------- */

/* ----------------------------------------
    instances or global variables
---------------------------------------- */
extern volatile SYSTIM systim;
extern volatile time_t unixTime;
extern Serial Serial1;  /* hardware serial 1 */

/* ----------------------------------------
    constructor destructor
---------------------------------------- */
SNTP::SNTP()
{
}

SNTP::SNTP( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp )
{
  begin( s, ip, dp, sp );
}

SNTP::~SNTP()
{
  end();
}

/* ----------------------------------------
    begin and end
---------------------------------------- */
void SNTP::begin( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp )
{
  soc = s;
  dip = ip;
  dport = dp;
  sport = sp;

  socOpen();
}

void SNTP::end()
{
  socClose();
}

/* ----------------------------------------
    socket open and close
---------------------------------------- */
void SNTP::refID( const uint8_t *ref )
{
  id[0] = ref[0];
  id[1] = ref[1];
  id[2] = ref[2];
  id[3] = ref[3];
}

/* ----------------------------------------
    socket open and close
---------------------------------------- */
void SNTP::socOpen()
{
  uint8_t status = getSn_SR( soc );
  if( status == SOCK_CLOSED ) socket( soc, Sn_MR_UDP, sport, 0 );
}

void SNTP::socClose()
{
  uint8_t status = getSn_SR( soc );
  if( status != SOCK_CLOSED ) close( soc );
}

/* ----------------------------------------
    ntp packet transmit for client
---------------------------------------- */
int32_t SNTP::transmit( uint32_t *ttH, uint32_t *ttL )
{
  sndNtpPacketFormat pkt;
  /* Leap Indicator(7,6),Version Number(5,4,3),Mode(2,1,0) */
  pkt.LiVnMode = NTP_FLAG_LI_NORMAL | NTP_FLAG_VN_3 | NTP_FLAG_MODE_CLIENT;
  pkt.Stratum = 0;              /* stratum */
  pkt.Poll = 0;                 /* poll interval */
  pkt.Precision = 0;            /* precision */
  pkt.rootDelay = 0UL;          /* root delay */
  pkt.rootDispersion = 0UL;     /* root dispersion */
  /* reference ID */
  pkt.referenceID[0] = id[0];
  pkt.referenceID[1] = id[1];
  pkt.referenceID[2] = id[2];
  pkt.referenceID[3] = id[3];
  pkt.referenceTimestampH = 0UL;  /* reference time */
  pkt.referenceTimestampL = 0UL;  /* reference time */
  pkt.originTimestampH = 0UL;   /* origin timestamp */
  pkt.originTimestampL = 0UL;   /* origin timestamp */
  pkt.receiveTimestampH = 0UL;  /* receive timestamp */
  pkt.receiveTimestampL = 0UL;  /* receive timestamp */

  pkt.transmitTimestampH = htonl( *ttH ); /* transmit timestamp */
  pkt.transmitTimestampL = htonl( *ttL ); /* transmit timestamp */

  return sendto( soc, (uint8_t *)&pkt, sizeof(pkt), (uint8_t *)dip, dport );
}

/* ----------------------------------------
    ntp packet recieve for client
---------------------------------------- */
int32_t SNTP::recieve(
  uint8_t *dip, uint16_t *port,
  uint32_t *ttH, uint32_t *ttL,
  uint8_t *stratum )
{
  uint16_t rcvLen;
  rcvNtpPacketFormat pkt;

  if( (rcvLen = getSn_RX_RSR( soc )) > 0 )
  {
    if( rcvLen > sizeof(pkt) ) rcvLen = sizeof(pkt);	// if Rx data size is lager sizeof(pkt)
    recvfrom( soc, (uint8_t *)&pkt, rcvLen, dip, port );
    *ttH = ntohl( pkt.transmitTimestampH );
    *ttL = ntohl( pkt.transmitTimestampL );
    *stratum = pkt.Stratum;

    return rcvLen;
  }
  return 0;
}


/* ----------------------------------------
    unix time get.
---------------------------------------- */
int SNTP::unix( time_t *second, uint16_t *milli )
{
  uint32_t t1H,t1L;
  t1H = 0UL;
  t1L = 0UL;
  uint32_t ret = transmit( &t1H, &t1L );
  if( ret > 0 )
  {
    SYSTIM baseTim = systim;
    while( true )
    {
      uint8_t dip[4];
      uint16_t port;
      uint32_t t4H,t4L;
      uint8_t stratum;
      ret = recieve( dip, &port, &t4H, &t4L, &stratum );
      if( ret > 0 && stratum < NTP_STRATUM_NOT_SYNC )
      {
        *second = (time_t)(t4H - UTC_EPOCH_DIFF);
        int32_t a; int32_t b = 0x40000000;
        a = t4L >> 2;  /* 0x3FFFFFFF */
        double t4D = (double)a / (double)b;
        *milli = (uint16_t)(t4D * 1000.0);
        return 0;
      }
      if( (systim - baseTim) >= 100UL ) break;
    }
  }

  return (-1);
}
