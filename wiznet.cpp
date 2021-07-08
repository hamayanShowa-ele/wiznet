/* ----------------------------------------
  wiznet SPI w5100,w5500 utilities
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
#include "wiznet.h"

/* ----------------------------------------
    prototypes
---------------------------------------- */

/* ----------------------------------------
    instances or global variables
---------------------------------------- */
extern volatile SYSTIM systim;

#if 0
extern uint8_t MAC[];      // self hardware address
extern uint8_t IPVAL[];    // self ip address
extern uint8_t GATEWAY[];  // default gateway
extern uint8_t SUBNET[];   // sub net mask
extern uint8_t DNS[];      // primary dns server
#endif

/* ----------------------------------------
    wizchip_select and wizchip_deselect
---------------------------------------- */
extern "C"
{
#if  (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200)
static uint8_t wizchip_read_SPI1( void ) { return spi_rw_SPI1( 0 ); }
static uint8_t wizchip_read_SPI2( void ) { return spi_rw_SPI2( 0 ); }
static uint8_t wizchip_read_SPI3( void ) { return spi_rw_SPI3( 0 ); }

static void wizchip_write_SPI1( uint8_t wb ) { spi_rw_SPI1( wb ); }
static void wizchip_write_SPI2( uint8_t wb ) { spi_rw_SPI2( wb ); }
static void wizchip_write_SPI3( uint8_t wb ) { spi_rw_SPI3( wb ); }

static void wizchip_spi_readburst_SPI1( uint8_t* pBuf, uint16_t len )
{
  for( ; len > 0; len-- ) *pBuf++ = wizchip_read_SPI1();
}
static void wizchip_spi_readburst_SPI2( uint8_t* pBuf, uint16_t len )
{
  for( ; len > 0; len-- ) *pBuf++ = wizchip_read_SPI2();
}
static void wizchip_spi_readburst_SPI3( uint8_t* pBuf, uint16_t len )
{
  for( ; len > 0; len-- ) *pBuf++ = wizchip_read_SPI3();
}

static void wizchip_spi_writeburst_SPI1( uint8_t* pBuf, uint16_t len )
{
  for( ; len > 0; len-- ) wizchip_write_SPI1( *pBuf++ );
}
static void wizchip_spi_writeburst_SPI2( uint8_t* pBuf, uint16_t len )
{
  for( ; len > 0; len-- ) wizchip_write_SPI2( *pBuf++ );
}
static void wizchip_spi_writeburst_SPI3( uint8_t* pBuf, uint16_t len )
{
  for( ; len > 0; len-- ) wizchip_write_SPI3( *pBuf++ );
}

static void cris_en_SPI1( void ){ waiSema_SPI1(); }
static void cris_en_SPI2( void ){ waiSema_SPI2(); }
static void cris_en_SPI3( void ){ waiSema_SPI3(); }

static void cris_ex_SPI1( void ){ sigSema_SPI1(); }
static void cris_ex_SPI2( void ){ sigSema_SPI2(); }
static void cris_ex_SPI3( void ){ sigSema_SPI3(); }

#elif  (_WIZCHIP_ == 5300)

static iodata_t wizchip_read( uint32_t addr ) { return *((volatile iodata_t *)addr); }
static void wizchip_write( uint32_t addr, iodata_t wb ) { *((volatile iodata_t *)addr) = wb; }
static void cris_en_W5300( void ){ wai_sem( SEMID_W5300 ); }
static void cris_ex_W5300( void ){ sig_sem( SEMID_W5300 ); }

#endif  /* (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200) */

}  /* extern "C" */

/* ----------------------------------------
    constructor destructor
---------------------------------------- */
WIZNET::WIZNET()
{
  clearNetworkInfo();
}

WIZNET::~WIZNET()
{
}

/* ----------------------------------------
    init and begin and end
---------------------------------------- */
void WIZNET::clearNetworkInfo()
{
  memset( _MAC, 0, sizeof(_MAC) );  // self hardware address
  memset( _IPVAL, 0, sizeof(_IPVAL) );  // self ip address
  memset( _GATEWAY, 0, sizeof(_GATEWAY) );  // default gateway
  memset( _SUBNET, 0, sizeof(_SUBNET) );  // sub net mask
  memset( _DNS, 0, sizeof(_DNS) );      // primary dns server
#if  (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200)
  cs_select = 0;
  cs_deselect = 0;
#elif  (_WIZCHIP_ == 5300)
#endif  /* (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200) */
}

#if  (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200)
void WIZNET::begin(
  SPI *wire, const uint8_t *memsize, void (*_s)(), void (*_d)(), dhcp_mode dhcp )
{
  spi = wire;
  cs_select = _s; cs_deselect = _d;

  /* wizchip initialize */
  (*cs_deselect)();  //  wizchip_deselect();
  reg_wizchip_cs_cbfunc( cs_select, cs_deselect );  //  reg_wizchip_cs_cbfunc( wizchip_select, wizchip_deselect );

  if( spi->whatTypeDef() == SPI1 )
  {
    reg_wizchip_spi_cbfunc( wizchip_read_SPI1, wizchip_write_SPI1 );
    reg_wizchip_spiburst_cbfunc( wizchip_spi_readburst_SPI1, wizchip_spi_writeburst_SPI1 );
    reg_wizchip_cris_cbfunc( cris_en_SPI1, cris_ex_SPI1 );
  }
  else if( spi->whatTypeDef() == SPI2 )
  {
    reg_wizchip_spi_cbfunc( wizchip_read_SPI2, wizchip_write_SPI2 );
    reg_wizchip_spiburst_cbfunc( wizchip_spi_readburst_SPI2, wizchip_spi_writeburst_SPI2 );
    reg_wizchip_cris_cbfunc( cris_en_SPI2, cris_ex_SPI2 );
  }
  else
  {
    reg_wizchip_spi_cbfunc( wizchip_read_SPI3, wizchip_write_SPI3 );
    reg_wizchip_spiburst_cbfunc( wizchip_spi_readburst_SPI3, wizchip_spi_writeburst_SPI3 );
    reg_wizchip_cris_cbfunc( cris_en_SPI3, cris_ex_SPI3 );
  }

  /* The order of the memsize array is sending and receiving. */
  if( ctlwizchip( CW_INIT_WIZCHIP, (void*)memsize ) == -1 )
  {
//    printf( "WIZCHIP Initialized fail.\r\n" );
    while( true ) rot_rdq();
  }
  dly_tsk( 2UL );

  /* wizchip software reset. */
  wizchip_sw_reset();
#if  _WIZCHIP_ == 5500
  /* check wizchip chip version. */
  if( getVERSIONR() != _CHIP_VERSION_ ) while( true ) rot_rdq();
#endif  /* _WIZCHIP_ == 5500 */

  /* retry count and period settings. */
  wiz_NetTimeout nettime = { 10, 200 * 10 };  /* number of resends, resend interval */
  wizchip_settimeout( &nettime );
//  wizchip_gettimeout( &nettime );

  /* Network information settings. */
  setNetworkInformations( dhcp );
}
#elif  (_WIZCHIP_ == 5300)
void WIZNET::begin( const uint8_t *memsize, dhcp_mode dhcp )
{
  //  reg_wizchip_bus_cbfunc( 0, 0 );
  reg_wizchip_bus_cbfunc( wizchip_read, wizchip_write );
  reg_wizchip_cris_cbfunc( cris_en_W5300, cris_ex_W5300 );

  /* The order of the memsize array is sending and receiving. and wizchip software reset. */
  if( ctlwizchip( CW_INIT_WIZCHIP, (void*)memsize ) == -1 )
  {
//    printf( "WIZCHIP Initialized fail.\r\n" );
    while( true ) rot_rdq();
  }
  dly_tsk( 2UL );

  /* check wizchip chip version. */
  uint16_t w5300_id;
  if( (w5300_id = getIDR()) != 0x5300 )
  {
    while( true ) rot_rdq();  // _CHIP_VERSION_
  }
  (void)w5300_id;

  /* retry count and period settings. */
  wiz_NetTimeout nettime = { 10, 200 * 10 };  /* number of resends, resend interval */
  wizchip_settimeout( &nettime );
//  wizchip_gettimeout( &nettime );

  /* Network information settings. */
  setNetworkInformations( dhcp );
}
#endif  /* (_WIZCHIP_ == 5500 || _WIZCHIP_ == 5100 || _WIZCHIP_ == 5200) */

/* ----------------------------------------
    network informations aet.
---------------------------------------- */
void WIZNET::setNetworkInformations( dhcp_mode dhcp )
{
  wiz_NetInfo pnetinfo;

  memcpy( pnetinfo.mac, _MAC, sizeof(pnetinfo.mac) );
  memcpy( pnetinfo.ip, _IPVAL, sizeof(pnetinfo.ip) );
  memcpy( pnetinfo.sn, _SUBNET, sizeof(pnetinfo.sn) );
  memcpy( pnetinfo.gw, _GATEWAY, sizeof(pnetinfo.gw) );
  memcpy( pnetinfo.dns, _DNS, sizeof(pnetinfo.dns) );
  pnetinfo.dhcp = dhcp;  /* 1:NETINFO_STATIC 2:NETINFO_DHCP */
  wizchip_setnetinfo( &pnetinfo );

  /* Check the settings. */
  wizchip_getnetinfo( &pnetinfo );
  if( memcmp( _MAC, pnetinfo.mac, sizeof(pnetinfo.mac) ) != 0 ) while( true ) rot_rdq();
  if( memcmp( _IPVAL, pnetinfo.ip, sizeof(pnetinfo.ip) ) != 0 ) while( true ) rot_rdq();
  if( memcmp( _SUBNET, pnetinfo.sn, sizeof(pnetinfo.sn) ) != 0 ) while( true ) rot_rdq();
  if( memcmp( _GATEWAY, pnetinfo.gw, sizeof(pnetinfo.gw) ) != 0 ) while( true ) rot_rdq();
  if( memcmp( _DNS, pnetinfo.dns, sizeof(pnetinfo.dns) ) != 0 ) while( true ) rot_rdq();
}

/* ----------------------------------------
    set mac,ip,gateway,subnet,dns address
---------------------------------------- */
void WIZNET::setMac( const uint8_t *src )
{
  memcpy( _MAC, src, sizeof(_MAC) );
}

void WIZNET::setIp( const uint8_t *src )
{
  memcpy( _IPVAL, src, sizeof(_IPVAL) );
}

void WIZNET::setGateway( const uint8_t *src )
{
  memcpy( _GATEWAY, src, sizeof(_GATEWAY) );
}

void WIZNET::setSubnet( const uint8_t *src )
{
  memcpy( _SUBNET, src, sizeof(_SUBNET) );
}

void WIZNET::setDns( const uint8_t *src )
{
  memcpy( _DNS, src, sizeof(_DNS) );
}


/* ----------------------------------------
    get mac,ip,gateway,subnet,dns address
---------------------------------------- */
void WIZNET::getMac( uint8_t *dest )
{
  memcpy( dest, _MAC, sizeof(_MAC) );
}

void WIZNET::getIp( uint8_t *dest )
{
  memcpy( dest, _IPVAL, sizeof(_IPVAL) );
}

void WIZNET::getGateway( uint8_t *dest )
{
  memcpy( dest, _GATEWAY, sizeof(_GATEWAY) );
}

void WIZNET::getSubnet( uint8_t *dest )
{
  memcpy( dest, _SUBNET, sizeof(_SUBNET) );
}

void WIZNET::getDns( uint8_t *dest )
{
  memcpy( dest, _DNS, sizeof(_DNS) );
}




/* ----------------------------------------
    wiznet socket begin
---------------------------------------- */
void WIZNET_SOCKET::begin( uint8_t s, const uint8_t *ip, uint16_t dp, uint16_t sp )
{
  soc = s;
  memcpy( dip, ip, sizeof(dip) );
  dport = dp;
  sport = sp;
  userFunction = nullptr;
}

void WIZNET_SOCKET::begin( uint8_t s, uint16_t sp )
{
  soc = s;
  sport = sp;
  userFunction = nullptr;
}

/* ----------------------------------------
    begin and end
---------------------------------------- */
void WIZNET_SOCKET::end()
{
  Close();
}

/* ----------------------------------------
    socket open
    return value :
      SOCK NUMBER : Result is OK about socket process.
      SOCKERR_SOCKNUM : Invalid socket number.
      SOCKERR_SOCKMODE : Not support socket mode as TCP, UDP, and so on.
      SOCKERR_SOCKFLAG : Invaild socket flag.
      SOCKERR_TIMEOUT : Timeout occurred.
---------------------------------------- */
int8_t WIZNET_SOCKET::Open( uint8_t mode, uint8_t flg, SYSTIM tmout )
{
  for( int i = 0; i < 10; i++ )
  {
    uint8_t status = getSn_SR( soc );
    if( status == SOCK_CLOSED )
    {
      int8_t ret = socket( soc, mode, sport, flg );  // mode = Sn_MR_TCP or Sn_MR_UDP, set flag = 0 or flag = SF_TCP_NODELAY
      return ret;
    }
    close( soc );
    dly_tsk( tmout );
  }
  return SOCKERR_TIMEOUT;
}

/* ----------------------------------------
    socket close
    return value :
      SOCK_OK : Result is OK about socket process.
      SOCKERR_SOCKNUM : Invalid socket number.
---------------------------------------- */
int8_t WIZNET_SOCKET::Close()
{
  return close( soc );
}

/* ----------------------------------------
    socket listen
    return value :
      SOCK_OK : Result is OK about socket process.
      SOCKERR_SOCKINIT : Socket is not initialized.
      SOCKERR_SOCKCLOSED : Socket closed unexpectedly.
---------------------------------------- */
int8_t WIZNET_SOCKET::Listen()
{
  return listen( soc );
}

/* ----------------------------------------
    socket connect
    return value :
      SOCK_OK : Result is OK about socket process.
      SOCKERR_SOCKNUM : Invalid socket number.
      SOCKERR_SOCKMODE : Not support socket mode as TCP, UDP, and so on.
      SOCKERR_SOCKFLAG : Invaild socket flag.
      SOCKERR_SOCKINIT : Socket is not initialized.
      SOCKERR_IPINVALID : Wrong server IP address.
      SOCKERR_PORTZERO : Server port zero.
      SOCKERR_TIMEOUT : Timeout occurred.
      SOCK_BUSY : In non-block io mode, it returned immediately.
---------------------------------------- */
int8_t WIZNET_SOCKET::Connect()
{
  int8_t ret = connect( soc, dip, dport );
  if( ret == SOCK_OK )
  {
    Getsockopt( SO_DESTIP, destIP );
    Getsockopt( SO_DESTPORT, &destPort );
  }
  return ret;
}

/* ----------------------------------------
    socket disconnect
    return value :
      SOCK_OK : Result is OK about socket process.
      SOCKERR_SOCKNUM : Invalid socket number.
      SOCKERR_SOCKMODE : Not support socket mode as TCP, UDP, and so on.
      SOCKERR_TIMEOUT : Timeout occurred.
      SOCK_BUSY : In non-block io mode, it returned immediately.
---------------------------------------- */
int8_t WIZNET_SOCKET::Disconnect()
{
  return disconnect( soc );
}

/* ----------------------------------------
    tcp packet transmit
    return value :
      0 > transmit data size.
      SOCKERR_SOCKSTATUS : Invalid socket status for socket operation.
      SOCKERR_TIMEOUT : Timeout occurred.
      SOCKERR_SOCKMODE : Invalid operation in the socket.
      SOCKERR_SOCKNUM : Invalid socket number.
      SOCKERR_DATALEN : zero data length.
      SOCK_BUSY : Socket is busy.
 ---------------------------------------- */
int32_t WIZNET_SOCKET::Send( const uint8_t *data, int32_t len )
{
  int32_t ret;
  int32_t length = len;
  uint16_t maxBufferSize;
  CtlSocket( CS_GET_MAXTXBUF, &maxBufferSize );
  uint8_t tempUC = SOCK_IO_NONBLOCK;  // SOCK_IO_NONBLOCK or SOCK_IO_BLOCK
  CtlSocket( CS_SET_IOMODE, &tempUC );

  while( length > 0 )
  {
    uint16_t l = (length > maxBufferSize) ? maxBufferSize : (uint16_t)length;
    ret = send( soc, (uint8_t *)data, l );
    if( ret > 0 )
    {
      data += ret;
      length -= ret;
      rot_rdq();
    }
    else if( ret == SOCK_BUSY )
    {
      dly_tsk( 2UL );
    }
    else
    {
      tempUC = SOCK_IO_BLOCK;
      CtlSocket( CS_SET_IOMODE, &tempUC );
      return ret;
    }
  }
  tempUC = SOCK_IO_BLOCK;
  CtlSocket( CS_SET_IOMODE, &tempUC );
  return (int32_t)len;
}

/* ----------------------------------------
    tcp packet recieve
    return value :
      0 > recieve data size.
      SOCKERR_SOCKSTATUS : Invalid socket status for socket operation.
      SOCKERR_SOCKMODE : Invalid operation in the socket.
      SOCKERR_SOCKNUM : Invalid socket number.
      SOCKERR_DATALEN : zero data length.
      SOCK_BUSY : Socket is busy.
      SOCKERR_TIMEOUT : Timeout occurred.
---------------------------------------- */
int32_t WIZNET_SOCKET::Recv( uint8_t *data, int32_t len, SYSTIM tmout )
{
  int32_t ret;
  int32_t length = len;
  int32_t rcvCount = 0;
  uint16_t maxBufferSize;
  CtlSocket( CS_GET_MAXRXBUF, &maxBufferSize );
  uint8_t tempUC = SOCK_IO_NONBLOCK;  // SOCK_IO_NONBLOCK or SOCK_IO_BLOCK
  CtlSocket( CS_SET_IOMODE, &tempUC );
  SYSTIM baseTim = systim;

  while( length > 0 )
  {
    uint16_t l = (length > maxBufferSize) ? maxBufferSize : (uint16_t)length;
    ret = recv( soc, data, l );
    if( ret > 0 )
    {
      data += ret;
      length -= ret;
      rcvCount += ret;
      baseTim = systim;
      if( tmout == 0 )
      {
        len = rcvCount;
        break;
      }
    }
    else if( ret == SOCKERR_DATALEN || ret == SOCK_BUSY )
    {
      if( (systim - baseTim) >= tmout )
      {
        len = (rcvCount) ? rcvCount : SOCKERR_TIMEOUT;
        break;
      }
    }
    else  // ex. SOCKERR_SOCKSTATUS or ...
    {
      len = ret;
      break;
    }
    dly_tsk( 2UL );
  }
  tempUC = SOCK_IO_BLOCK;
  CtlSocket( CS_SET_IOMODE, &tempUC );
  if( len > 0 )
  {
    Getsockopt( SO_DESTIP, destIP );
    Getsockopt( SO_DESTPORT, &destPort );
  }
  return (int32_t)len;
}

/* ----------------------------------------
    udp packet transmit
    return value :
      0 > transmit data size.
      SOCKERR_SOCKNUM : Invalid socket number \n
      SOCKERR_SOCKMODE : Invalid operation in the socket \n
      SOCKERR_SOCKSTATUS : Invalid socket status for socket operation \n
      SOCKERR_DATALEN : zero data length \n
      SOCKERR_IPINVALID : Wrong server IP address\n
      SOCKERR_PORTZERO : Server port zero\n
      SOCKERR_SOCKCLOSED : Socket unexpectedly closed \n
      SOCKERR_TIMEOUT : Timeout occurred \n
      SOCK_BUSY : Socket is busy.
 ---------------------------------------- */
int32_t WIZNET_SOCKET::Sendto( const uint8_t *data, int32_t len )
{
  int32_t ret;
  int32_t length = len;
  uint16_t maxBufferSize;
  CtlSocket( CS_GET_MAXTXBUF, &maxBufferSize );
  uint8_t tempUC = SOCK_IO_NONBLOCK;  // SOCK_IO_NONBLOCK or SOCK_IO_BLOCK
  CtlSocket( CS_SET_IOMODE, &tempUC );

  while( length > 0 )
  {
    uint16_t l = (length > 8192L) ? 8192 : (uint16_t)length;
    ret = sendto( soc, (uint8_t *)data, l, dip, dport );
    if( ret > 0 )
    {
      data += ret;
      length -= ret;
      rot_rdq();
    }
    else if( ret == SOCK_BUSY )
    {
      dly_tsk( 2UL );
    }
    else
    {
      tempUC = SOCK_IO_BLOCK;
      CtlSocket( CS_SET_IOMODE, &tempUC );
      return ret;
    }
  }
  tempUC = SOCK_IO_BLOCK;
  CtlSocket( CS_SET_IOMODE, &tempUC );

  return (int32_t)len;
}


/* ----------------------------------------
    udp packet recieve
    return value :
      0 > recieve data size.
      SOCKERR_DATALEN : zero data length.
      SOCKERR_SOCKMODE : Invalid operation in the socket.
      SOCKERR_SOCKNUM : Invalid socket number.
      SOCK_BUSY : Socket is busy.
---------------------------------------- */
int32_t WIZNET_SOCKET::Recvfrom( uint8_t *data, int32_t len, SYSTIM tmout )
{
  int32_t ret;
  int32_t length = len;
  uint16_t maxBufferSize;
  CtlSocket( CS_GET_MAXRXBUF, &maxBufferSize );
  uint8_t tempUC = SOCK_IO_NONBLOCK;  // SOCK_IO_NONBLOCK or SOCK_IO_BLOCK
  CtlSocket( CS_SET_IOMODE, &tempUC );
  SYSTIM baseTim = systim;

  while( length > 0 )
  {
    uint16_t l = (length > maxBufferSize) ? maxBufferSize : (uint16_t)length;
    ret = recvfrom( soc, data, l, destIP, &destPort );
    if( ret > 0 )
    {
      data += ret;
      length -= ret;
      baseTim = systim;
    }
    else if( ret == SOCKERR_DATALEN || ret == SOCK_BUSY )
    {
      if( (systim - baseTim) >= tmout )
      {
        len = SOCKERR_TIMEOUT;
        break;
      }
    }
    dly_tsk( 2UL );
  }
  tempUC = SOCK_IO_BLOCK;
  CtlSocket( CS_SET_IOMODE, &tempUC );
  return len;
}

/* ----------------------------------------
    control socket
    parameters :
      sn : socket number
      cstype : type of control socket. refer to @ref ctlsock_type.
        CS_SET_IOMODE,CS_GET_IOMODE
          uint8_t SOCK_IO_BLOCK,SOCK_IO_NONBLOCK
        CS_GET_MAXTXBUF,CS_GET_MAXRXBUF
          uint16_t 0 ~ 16K
        CS_CLR_INTERRUPT,CS_GET_INTERRUPT,CS_SET_INTMASK,CS_GET_INTMASK
          sockint_kind : SIK_CONNECTED, etc.
      arg : Data type and value is determined according to @ref ctlsock_type. \n
    return value :
      SOCK_OK
      SOCKERR_ARG : Invalid argument.
---------------------------------------- */
int8_t WIZNET_SOCKET::CtlSocket( ctlsock_type cstype, void* arg )
{
  return ctlsocket( soc, cstype, arg );
}

/* ----------------------------------------
    get socket options
    parameters :
      sn : socket number
      sotype : data type and
        SO_FLAG : uint8_t,SF_ETHER_OWN, etc...
        SO_TOS : uint8_t,0 ~ 255
        SO_MSS : uint16_t,0 ~ 65535
        SO_DESTIP : uint8_t[4],
        SO_DESTPORT : uint16_t,
        SO_KEEPALIVEAUTO : uint8_t,0 ~ 255
        SO_SENDBUF : uint16_t,0 ~ 65535
        SO_RECVBUF : uint16_t,0 ~ 65535
        SO_STATUS : uint8_t,SOCK_ESTABLISHED, etc..
        SO_REMAINSIZE : uint16_t,0~ 65535
        SO_PACKINFO : uint8_t,PACK_FIRST, etc...
    return value :
      SOCK_OK
      SOCKERR_SOCKNUM : Invalid Socket number
      SOCKERR_SOCKOPT : Invalid socket option or its value
      SOCKERR_SOCKMODE : Invalid socket mode
---------------------------------------- */
int8_t WIZNET_SOCKET::Getsockopt( sockopt_type sotype, void* arg )
{
  return getsockopt( soc, sotype, arg );
}

/* ----------------------------------------
    rcp shut down
---------------------------------------- */
void WIZNET_SOCKET::shutDown()
{
  Disconnect();
  while( 1 )
  {
    uint8_t status;
    Getsockopt( SO_STATUS, &status );
    if( status == SOCK_CLOSED ) break;
  }
}

#if 0
/* ----------------------------------------
    server
    parameters :
      _f : user function called in established.
---------------------------------------- */
int WIZNET_SOCKET::Server( void(*_f)() )
{
  if( userFunction != _f ){ userFunction = _f; }

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
      (*userFunction)();
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
#endif
