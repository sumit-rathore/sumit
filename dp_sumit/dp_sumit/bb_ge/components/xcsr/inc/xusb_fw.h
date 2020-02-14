///////////////////////////////////////////////////////////////////////////////
//
//   Icron Technology Corporation - Copyright 2009,2010,2011
//
///////////////////////////////////////////////////////////////////////////////
//
//   Title       :  xusb.vh
//   Created     :  09-06-08
//   Author      :  kenm
//
///////////////////////////////////////////////////////////////////////////////
//
//   Description :
//
//   Common xusb definitions
//
///////////////////////////////////////////////////////////////////////////////
//
//     $Id: xusb.vh,v 1.22 2014/06/16 09:29:34 terrys Exp $
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
// Static QIDs
//-----------------------------------------------------------------------------

  //         RESERVED               0 // must define as "= decimal"
  #define  SQL_SOF                1 // for software conversion
  #define  SQ_ASYNC               2 // parsing script
  #define  SQ_PERIODIC            3
  #define  SQ_ACC_BLK             4 // be sure to update counts
  #define  SQL_CPU_USB_CTRL       5 // when you add or remove
  #define  SQR_CPU_DEV_RESP       5 // a static QID.
  #define  SQ_CPU_TX_USB          6
  #define  SQ_CPU_TX              7
  #define  SQ_CPU_RX              8
  #define  SQ_CPU_TX_ENET         9
  #define  SQ_CPU_RX_ENET         10
  #define  SQ_QOS                 11
  #define  SQL_USB_RTY0           12
  #define  SQR_HSCH               12
  #define  SQL_USB_RTY1           13
  #define  SQR_SCH_ASYNC_INB      13
  #define  SQR_SCH_PERIODIC       14
  #define  SQL_CPU_RSP_QID        14
  #define  SQR_SCH_MSA            15
  #define  SQR_SCH_ASYNC_OTB      16
  #define  SQR_SCH_UFRM_P0        17
  #define  SQR_SCH_UFRM_P1        18
  #define  SQR_SCH_UFRM00         19
  #define  SQR_SCH_UFRM01         20
  #define  SQR_SCH_UFRM02         21
  #define  SQR_SCH_UFRM03         22
  #define  SQR_SCH_UFRM04         23
  #define  SQR_SCH_UFRM05         24
  #define  SQR_SCH_UFRM06         25
  #define  SQR_SCH_UFRM07         26
  #define  SQR_SCH_UFRM10         27
  #define  SQR_SCH_UFRM11         28
  #define  SQR_SCH_UFRM12         29
  #define  SQR_SCH_UFRM13         30
  #define  SQR_SCH_UFRM14         31
  #define  SQR_SCH_UFRM15         32
  #define  SQR_SCH_UFRM16         33
  #define  SQR_SCH_UFRM17         34
  #define  SQR_MSA_RETRY          35
  #define  SQR_CPU_DEV_RESP_DATA  36

  #define         SQL_COUNT              15
  #define         SQR_COUNT              37

//-----------------------------------------------------------------------------
// Static QID Types
//-----------------------------------------------------------------------------

  #define  QT_DEF                 0
  #define  QT_DNS_ASYNC           1
  #define  QT_DNS_PERIODIC        2
  #define  QT_DNS_ACCBULK         3
  #define  QT_UPS_ASYNC           4
  #define  QT_UPS_PERIODIC        5
  #define  QT_UPS_ACCBULK_VP1     6
  #define  QT_UPS_ACCBULK_VP2     7
  #define  QT_UPS_ACCBULK_VP3     8
  #define  QT_UPS_ACCBULK_VP4     9
  #define  QT_UPS_ACCBULK_VP5     10
  #define  QT_UPS_ACCBULK_VP6     11
  #define  QT_UPS_ACCBULK_VP7     12

//-----------------------------------------------------------------------------
// End Software Parsing Command
//-----------------------------------------------------------------------------

// ___ICRON_FW_END___
