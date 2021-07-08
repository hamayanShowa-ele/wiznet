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
#include "ntp.h"

/* ----------------------------------------
    prototypes 
---------------------------------------- */

/* ----------------------------------------
    instances or global variables
---------------------------------------- */
extern volatile SYSTIM systim;
extern volatile time_t unixTime;
extern volatile uint32_t pps_millisecond_counter;
extern volatile bool gpsLock;

extern STM32F_TIMER tim1ms;
extern STM32F_TIMER tim1s;
extern Serial Serial1;  /* hardware serial 1 */

/* ----------------------------------------
    ntp Calculating a Fewer Digits
---------------------------------------- */
static uint32_t ntpFewerDigits( double few )
{
  int32_t a = (int32_t)(few * 1000000000.0);
  int32_t b = 1000000000L / 2;
  uint32_t bits = 0x00000000;
  int i;
  for( i = 0; i < 32; i++ )
  {
    if( a == b )
    {
      bits |= 0x00000001;
      break;
    }
    else if( a > b )
    {
      bits |= 0x00000001;
      a -= b;
    }
    else {}
    b /= 2L;
    if( b == 0L ) break;
    bits <<= 1;
  }
  bits <<= 31 - i;
  return bits;
}

/* ----------------------------------------
    ntp Get a few digits from the timer.
---------------------------------------- */
static double fewDigitFromTimer( uint32_t *sec )
{
  loc_cpu();
  uint16_t us = tim1ms.getCounter();
  uint16_t ar = tim1ms.getAutoReload();
//  uint16_t ms = tim1s.getCounter();
  uint16_t ms = pps_millisecond_counter;
  uint32_t gmt = (uint32_t)unixTime;
  unl_cpu();
  gmt += UTC_EPOCH_DIFF;
  /* microsecond */
  double a = ((double)us / (double)(ar + 1)) / 1000.0;
  /* millisecond */
  a += (double)ms / 1000.0;
#if 1
  /* offset calibration. */
  double b = 0.001;
  if( a >= b ) a -= b;
  else
  {
    gmt--;
    a = a + 1.0 - b;
  }
#endif
  *sec = gmt;

  return a;
}

/* ----------------------------------------
    constructor destructor
---------------------------------------- */
NTP::NTP()
{
}

NTP::NTP( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp )
{
  begin( s, ip, dp, sp );
}

NTP::NTP( uint8_t s, uint16_t sp )
{
  begin( s, sp );
}

NTP::~NTP()
{
  end();
}

/* ----------------------------------------
    begin and end
---------------------------------------- */
void NTP::begin( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp )
{
  soc = s;
  memcpy( dip, ip, sizeof(dip) );
  dport = dp;
  sport = sp;
  for( int i = 0; i < (int)(sizeof(ave) / sizeof(ave[0])); i++ ) ave[i] = 0;
  id[0] = 'h'; id[1] = 'a'; id[2] = 'm'; id[3] = 'a';
  wptr = 0;

  socOpen();
}

void NTP::begin( uint8_t s, uint16_t sp )
{
  soc = s;
//  sip = ip;
  sport = sp;
  for( int i = 0; i < (int)(sizeof(ave) / sizeof(ave[0])); i++ ) ave[i] = 0;
  wptr = 0;

  socOpen();
}

void NTP::end()
{
  socClose();
}

/* ----------------------------------------
    socket open and close
---------------------------------------- */
void NTP::refID( const uint8_t *ref )
{
  id[0] = ref[0];
  id[1] = ref[1];
  id[2] = ref[2];
  id[3] = ref[3];
}

/* ----------------------------------------
    socket open and close
---------------------------------------- */
void NTP::socOpen()
{
  uint8_t status = getSn_SR( soc );
  if( status == SOCK_CLOSED ) socket( soc, Sn_MR_UDP, sport, 0 );
}

void NTP::socClose()
{
  uint8_t status = getSn_SR( soc );
  if( status != SOCK_CLOSED ) close( soc );
}

/* ----------------------------------------
    ntp packet transmit for client
---------------------------------------- */
int32_t NTP::transmitC( uint32_t *ttH, uint32_t *ttL )
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

  *ttL = ntpFewerDigits( fewDigitFromTimer( ttH ) );
  pkt.transmitTimestampH = htonl( *ttH ); /* transmit timestamp */
  pkt.transmitTimestampL = htonl( *ttL ); /* transmit timestamp */

  return sendto( soc, (uint8_t *)&pkt, sizeof(pkt), dip, dport );
}

/* ----------------------------------------
    ntp packet recieve for client
---------------------------------------- */
int32_t NTP::recieveC(
  uint8_t *dip, uint16_t *port,
  uint32_t *otH, uint32_t *otL,
  uint32_t *rtH, uint32_t *rtL,
  uint32_t *ttH, uint32_t *ttL,
  uint8_t *stratum )
{
  uint16_t rcvLen;
  rcvNtpPacketFormat pkt;

  if( (rcvLen = getSn_RX_RSR( soc )) > 0 )
  {
    if( rcvLen > sizeof(pkt) ) rcvLen = sizeof(pkt);	// if Rx data size is lager sizeof(pkt)
    recvfrom( soc, (uint8_t *)&pkt, rcvLen, dip, port );
    *ttL = ntpFewerDigits( fewDigitFromTimer( ttH ) );

    *otH = ntohl( pkt.receiveTimestampH );
    *otL = ntohl( pkt.receiveTimestampL );
    *rtH = ntohl( pkt.transmitTimestampH );
    *rtL = ntohl( pkt.transmitTimestampL );
    *stratum = pkt.Stratum;

    return rcvLen;
  }
  return 0;
}

int32_t NTP::recieveC(
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
    ntp packet transmit for server
---------------------------------------- */
int32_t NTP::transmitS( sndNtpPacketFormat *pkt, const uint8_t *dip, uint16_t dport )
{
  /* Leap Indicator(7,6),Version Number(5,4,3),Mode(2,1,0) */
  pkt->LiVnMode = NTP_FLAG_LI_NORMAL | NTP_FLAG_VN_3 | NTP_FLAG_MODE_SERVER;
  /* stratum NTP_STRATUM_NOT_SYNC or NTP_STRATUM_FROM_GPS */
  pkt->Stratum = (gpsLock == true) ? NTP_STRATUM_FROM_GPS : NTP_STRATUM_NOT_SYNC;
//  pkt->Stratum = (gpsLock == true) ? NTP_STRATUM_2 : NTP_STRATUM_NOT_SYNC;
  pkt->Poll = 10;                /* poll interval */
  pkt->Precision = -10;          /* precision -2^10 = 1ms */
  pkt->rootDelay = 0UL;          /* root delay */
  pkt->rootDispersion = 0UL;     /* root dispersion */
  /* reference ID */
  if( pkt->Stratum == NTP_STRATUM_FROM_GPS )
  {
    pkt->referenceID[0] = 'G';
    pkt->referenceID[1] = 'P';
    pkt->referenceID[2] = 'S';
    pkt->referenceID[3] = '\0';
  }
  else
  {
    pkt->referenceID[0] = id[0];
    pkt->referenceID[1] = id[1];
    pkt->referenceID[2] = id[2];
    pkt->referenceID[3] = id[3];
  }
//  pkt.referenceTimestampH = 0UL;  /* reference time */
//  pkt.referenceTimestampL = 0UL;  /* reference time */
//  pkt->originTimestampH = 0UL;   /* origin timestamp */
//  pkt->originTimestampL = 0UL;   /* origin timestamp */
//  pkt->receiveTimestampH = 0UL;  /* receive timestamp */
//  pkt->receiveTimestampL = 0UL;  /* receive timestamp */
  uint32_t ttH,ttL;
  ttL = ntpFewerDigits( fewDigitFromTimer( &ttH ) );
  pkt->transmitTimestampH = htonl( ttH ); /* transmit timestamp */
  pkt->transmitTimestampL = htonl( ttL ); /* transmit timestamp */

  return sendto( soc, (uint8_t *)pkt, sizeof(sndNtpPacketFormat), (uint8_t *)dip, dport );
}


/* ----------------------------------------
    ntp packet recieve for server
---------------------------------------- */
int32_t NTP::recieveS( rcvNtpPacketFormat *pkt, uint8_t *dip, uint16_t *dport )
{
  uint16_t rcvLen;

  if( (rcvLen = getSn_RX_RSR( soc )) > 0 )
  {
    if( rcvLen > sizeof(rcvNtpPacketFormat) ) rcvLen = sizeof(rcvNtpPacketFormat);	// if Rx data size is lager sizeof(pkt)
    recvfrom( soc, (uint8_t *)pkt, rcvLen, dip, dport );
    uint32_t rtH,rtL;
    rtL = ntpFewerDigits( fewDigitFromTimer( &rtH ) );
    pkt->receiveTimestampH = htonl( rtH );   /* receive timestamp */
    pkt->receiveTimestampL = htonl( rtL );   /* receive timestamp */
    pkt->originTimestampH = pkt->transmitTimestampH;
    pkt->originTimestampL = pkt->transmitTimestampL;

    return rcvLen;
  }
  return 0;
}


/* ----------------------------------------
    unix time get.
---------------------------------------- */
int NTP::unix( time_t *second, uint16_t *milli )
{
  uint32_t t1H,t1L;
  t1H = unixTime;
  t1L = 0UL;
  uint32_t ret = transmitC( &t1H, &t1L );
  if( ret > 0 )
  {
    SYSTIM baseTim = systim;
    while( true )
    {
      uint8_t dip[4];
      uint16_t port;
      uint32_t t2H,t2L;
      uint32_t t3H,t3L;
      uint32_t t4H,t4L;
      uint8_t stratum;
      ret = recieveC( dip, &port, &t2H, &t2L, &t3H, &t3L, &t4H, &t4L, &stratum );
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
      rot_rdq();
    }
  }

  return (-1);
}

int NTP::unixSimple( time_t *second, uint16_t *milli )
{
  uint32_t t1H,t1L;
  t1H = unixTime;
  t1L = 0UL;
  uint32_t ret = transmitC( &t1H, &t1L );
  if( ret > 0 )
  {
    SYSTIM baseTim = systim;
    while( true )
    {
      uint8_t dip[4];
      uint16_t port;
      uint32_t t4H,t4L;
      uint8_t stratum;
      ret = recieveC( dip, &port, &t4H, &t4L, &stratum );
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
      rot_rdq();
    }
  }

  return (-1);
}

/* ----------------------------------------
    fixed size reload value.
---------------------------------------- */
#if 0
int NTP::fixedNewReload( double offset )
{
  if( offset > 0.20 ) return (-500);
  else if( offset > 0.10 ) return (-200);
  else if( offset > 0.05 ) return (-100);
  else if( offset > 0.02 ) return (-50);
  else if( offset > 0.01 ) return (-20);
  else if( offset > 0.005 ) return (-10);
  else if( offset > 0.002 ) return (-5);
  else if( offset > 0.001 ) return (-1);
  else if( offset < -0.20 ) return 500;
  else if( offset < -0.10 ) return 200;
  else if( offset < -0.05 ) return 100;
  else if( offset < -0.02 ) return 50;
  else if( offset < -0.01 ) return 20;
  else if( offset < -0.005 ) return 10;
  else if( offset < -0.002 ) return 5;
  else if( offset < -0.001 ) return 1;
  else ;
  return 0;
}
#endif

/*
    variable size reload value.
  ((36000 - y) / 36000 * 10 * 1000) - 10000 = x * 0.7
  ((36000 - y) / 36000 * 10 * 1000) = x * 0.7 + 10000
  ((36000 - y) * 10 * 1000) = (x * 0.7 + 10000) * 36000
  (36000 - y) = (x * 0.7 + 10000) * 36000 / (10 * 1000)
  -y = (x * 0.7 + 10000) * 36000 / (10 * 1000) - 36000
  y = 36000 - (x * 0.7 + 10000) * 36000 / (10 * 1000)
*/
int NTP::variableNewReload( double x )
{
  if( x >= 1.0 || x <= -1.0 ) return 0;
  x *= 1000.0;  /* convert to millisecond. */
  if( (int)x > 700 ) x = 700.0;
  else if( (int)x < -700 ) x = -700.0;
  else {}
  double y = 36000.0 - ((x * DAMPING_FACTOR) + 10000.0) * (36000.0 / 10000.0);
  int z = (int)(y + 0.5);
  if( z > 700 ) z = 700;
  else if( z < -700 ) z = -700;
  return z;
}


/* ----------------------------------------
    moving average.
---------------------------------------- */
int NTP::movingAverage( int a )
{
#if 1
  ave[ wptr ] = a;
  if( ++wptr >= (int)(sizeof(ave) / sizeof(ave[0])) ) wptr = 0;
  int b = 0;
  for( int i = 0; i < (int)(sizeof(ave) / sizeof(ave[0])); i++ ) b += ave[i];
  b /= sizeof(ave) / sizeof(ave[0]);
  if( b > 1 ) b = 1;
  else if( b < -1 ) b = -1;
  return b;
#else
  if( a < -5 || a > 5 ) return 0;
  ave[ a + 5 ]++;
  int max = 0;
  int total = ave[0];
  for( int i = 1; i < (int)(sizeof(ave) / sizeof(ave[0])); i++ )
  {
    if( ave[i] > ave[max]) max = i;
    total += ave[i];
  }
#if 0
  char buffer[64];
  sprintf( buffer, "    %d,%d,%d,%d,%d, %d, %d,%d,%d,%d,%d, %d\r\n",
    ave[0],ave[1],ave[2],ave[3],ave[4],ave[5],ave[6],ave[7],ave[8],ave[9],ave[10],max );
  Serial1.printf( buffer );
#endif

  return (total < 20) ? 0 : max - 5;
#endif
}

