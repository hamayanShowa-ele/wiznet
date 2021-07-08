/* ----------------------------------------
  sntp utilities
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

#ifndef _SNTP_H_
#define _SNTP_H_

#include  <Timer.h>
#include  <HardwareSerial.h>
#include  <strutil.h>

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
#define NTP_PORT        123     //ntp server port number
#define SECS_PERDAY     86400UL  // seconds in a day = 60*60*24
#define UTC_ADJ_HRS     9        // japan : GMT+9
#define EPOCH           1900     // NTP start year
#define YEAR_366_COUNT  ((1970 - EPOCH - 0) / 4)  //
#define YEAR_365_COUNT  ((1970 - EPOCH - 0) - YEAR_366_COUNT)  //
#define UTC_EPOCH_DIFF  (((YEAR_365_COUNT * 365) + (YEAR_366_COUNT * 366)) * SECS_PERDAY)
//#define DAMPING_FACTOR  0.7

#define NTP_FLAG_LI_NORMAL  (0 << 6)
#define NTP_FLAG_LI_61SEC   (1 << 6)
#define NTP_FLAG_LI_59SEC   (2 << 6)
#define NTP_FLAG_LI_UNKNOWN (3 << 6)
#define NTP_FLAG_VN_3       (3 << 3)
#define NTP_FLAG_VN_4       (4 << 3)
#define NTP_FLAG_MODE_SYMMETRIC_ACTIVE   (1 << 0)
#define NTP_FLAG_MODE_SYMMETRIC_PASSIVE  (2 << 0)
#define NTP_FLAG_MODE_CLIENT     (3 << 0)
#define NTP_FLAG_MODE_SERVER     (4 << 0)
#define NTP_FLAG_MODE_BROADCAST  (5 << 0)
#define NTP_FLAG_MODE_CONTROL    (6 << 0)
#define NTP_FLAG_MODE_PRIVATE    (7 << 0)

#define NTP_STRATUM_NOT_USED    (0)
#define NTP_STRATUM_FROM_GPS    (1)
#define NTP_STRATUM_2           (2)
#define NTP_STRATUM_NOT_SYNC    (16)

typedef struct _sndNtpPacketFormat  /* https://milestone-of-se.nesuke.com/l7protocol/ntp/ntp-format/ */
{
  uint8_t LiVnMode;  /* Leap Indicator(7,6),Version Number(5,4,3),Mode(2,1,0) */
  uint8_t Stratum;   /* Stratum */
  uint8_t Poll;      /* Poll */
  uint8_t Precision; /* Precision */
  uint32_t rootDelay; /* Root Delay */
  uint32_t rootDispersion; /* Root Dispersion */
  char     referenceID[4]; /* Reference ID */
  uint32_t referenceTimestampH; /* Reference Timestamp (A second digit) */
  uint32_t referenceTimestampL; /* Reference Timestamp (A few digits) */
  uint32_t originTimestampH; /* Origin Timestamp (A second digit) */
  uint32_t originTimestampL; /* Origin Timestamp (A few digits) */
  uint32_t receiveTimestampH; /* Receive Timestamp (A second digit) */
  uint32_t receiveTimestampL; /* Receive Timestamp (A few digits) */
  uint32_t transmitTimestampH; /* Transmit Timestamp (A second digit) */
  uint32_t transmitTimestampL; /* Transmit Timestamp (A few digits) */
} sndNtpPacketFormat;

typedef struct _rcvNtpPacketFormat  /* https://milestone-of-se.nesuke.com/l7protocol/ntp/ntp-format/ */
{
//  uint8_t dip[4];    /* destination ip address. */
  uint8_t LiVnMode;  /* Leap Indicator(7,6),Version Number(5,4,3),Mode(2,1,0) */
  uint8_t Stratum;   /* Stratum */
  uint8_t Poll;      /* Poll */
  uint8_t Precision; /* Precision */
  uint32_t rootDelay; /* Root Delay */
  uint32_t rootDispersion; /* Root Dispersion */
  uint8_t  referenceID[4]; /* Reference ID */
  uint32_t referenceTimestampH; /* Reference Timestamp (A second digit) */
  uint32_t referenceTimestampL; /* Reference Timestamp (A few digits) */
  uint32_t originTimestampH; /* Origin Timestamp (A second digit) */
  uint32_t originTimestampL; /* Origin Timestamp (A few digits) */
  uint32_t receiveTimestampH; /* Receive Timestamp (A second digit) */
  uint32_t receiveTimestampL; /* Receive Timestamp (A few digits) */
  uint32_t transmitTimestampH; /* Transmit Timestamp (A second digit) */
  uint32_t transmitTimestampL; /* Transmit Timestamp (A few digits) */
} rcvNtpPacketFormat;

class SNTP // : public hoge
{
private:
  uint8_t soc,id[4];
  const uint8_t *dip;
  uint16_t sport,dport;

public:
  SNTP();
  SNTP( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp = 0 );  // for ntp client.
  ~SNTP();

  void begin( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp = 0 );  // for ntp client.
  void end();
  void refID( const uint8_t *ref );

  void socOpen();
  void socClose();
  int32_t transmit( uint32_t *ttH, uint32_t *ttL );
  int32_t recieve(
    uint8_t *dip, uint16_t *port,
    uint32_t *ttH, uint32_t *ttL,
    uint8_t *stratum );

  int  unix( time_t *second, uint16_t *milli );
};

#endif  /* _SNTP_H_ */
