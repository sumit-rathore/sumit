///////////////////////////////////////////////////////////////////////////////
//
//   Icron Technology Corporation - Copyright 2009
//
///////////////////////////////////////////////////////////////////////////////
//
//   Title       :  xsst.vh
//   Created     :  July 28, 2009
//   Author      :  Terence C. Sosniak
//
///////////////////////////////////////////////////////////////////////////////
//
//   Description :
//     XSST Parameters and access functions.
//
//
///////////////////////////////////////////////////////////////////////////////
//
//     $Id: xsst.vh,v 1.36 2014/06/16 09:29:34 terrys Exp $
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


//
// *** Please Read the Software Script section below ***
//


///////////////////////////////////////////////////////////////////////////////
///////////////////////////                          //////////////////////////
///////////////////////////     XSST Bit Positions   //////////////////////////
///////////////////////////                          //////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// Description:
//   These #defines illustrate the xsst structure.
//
//////////////////////////////////////////////////////////////////////////////
#define XSST_RECORD_WIDTH          64 //does not include the Logical Address Table (LAT) width
//
// Common to all endpoints for a particular USB address.
//
#define XSST_XPORT_WIDTH           4
#define XSST_INSYS_WIDTH           1
#define XSST_SPLITDEV_WIDTH        1
#define XSST_VF_WIDTH              1
//
// General Data QID (in or out) management
//
#define XSST_DQID_WIDTH            7

//
// XSST Bitfield mappings
//
#define XSST_IBLK_OFFSET           63
#define XSST_IBLK_WIDTH            1

#define XSST_OBLK_OFFSET           62
#define XSST_OBLK_WIDTH            1

#define XSST_IERR_CNT_OFFSET       60
#define XSST_IERR_CNT_WIDTH        2

#define XSST_OERR_CNT_OFFSET       58
#define XSST_OERR_CNT_WIDTH        2

#define XSST_IFWD2CPU_OFFSET       56
#define XSST_IFWD2CPU_WIDTH        2

#define XSST_OFWD2CPU_OFFSET       54
#define XSST_OFWD2CPU_WIDTH        2

#define XSST_OVRLAY_OFFSET         46
#define XSST_OVRLAY_WIDTH          8

#define XSST_ISPLIT_OFFSET         45
#define XSST_ISPLIT_WIDTH          1

#define XSST_OSPLIT_OFFSET         44
#define XSST_OSPLIT_WIDTH          1

#define XSST_IDCF_OFFSET           43
#define XSST_IDCF_WIDTH            1

#define XSST_ODCF_OFFSET           42
#define XSST_ODCF_WIDTH            1

#define XSST_IEPTYPESEL_OFFSET     41
#define XSST_IEPTYPESEL_WIDTH      1

#define XSST_OEPTYPESEL_OFFSET     40
#define XSST_OEPTYPESEL_WIDTH      1

#define XSST_RTRY_USAGE_OFFSET     38
#define XSST_RTRY_USAGE_WIDTH      2

#define XSST_IFWD2CPUGATE_OFFSET   37//when fwd2cpu is not block DNS & UPS directions this is ALTCLRRSP bit for overide
#define XSST_IFWD2CPUGATE_WIDTH    1

#define XSST_OFWD2CPUGATE_OFFSET   36//when fwd2cpu is not block DNS & UPS directions this is ALTCLRRSP bit for overide
#define XSST_OFWD2CPUGATE_WIDTH    1

#define XSST_ICLR_OFFSET           35
#define XSST_ICLR_WIDTH            1

#define XSST_OCLR_OFFSET           34
#define XSST_OCLR_WIDTH            1

#define XSST_IHST_ACS_OFFSET       33
#define XSST_IHST_ACS_WIDTH        1

#define XSST_OHST_ACS_OFFSET       32
#define XSST_OHST_ACS_WIDTH        1

#define XSST_IDET_OFFSET           31
#define XSST_IDET_WIDTH            1

#define XSST_ODET_OFFSET           30
#define XSST_ODET_WIDTH            1

#define XSST_IQID_OFFSET           23
#define XSST_IQID_WIDTH            7

#define XSST_OQID_OFFSET           16
#define XSST_OQID_WIDTH            7

#define XSST_INTFYCNT_OFFSET       8
#define XSST_INTFYCNT_WIDTH        8

#define XSST_ONTFYCNT_OFFSET       6
#define XSST_ONTFYCNT_WIDTH        2

#define XSST_IACCELMODE_OFFSET     5
#define XSST_IACCELMODE_WIDTH      1

#define XSST_OACCELMODE_OFFSET     4
#define XSST_OACCELMODE_WIDTH      1

#define XSST_IEPTYPE_OFFSET        2
#define XSST_IEPTYPE_WIDTH         2

#define XSST_OEPTYPE_OFFSET        0
#define XSST_OEPTYPE_WIDTH         2

/////////////////////////////////////////////////////////////////////////////////
///////////////////////////                          ////////////////////////////
///////////////////////////          LAT             ////////////////////////////
///////////////////////////                          ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//   For software interface, this descibes the position of the bits in the
//   registers sets.
//
////////////////////////////////////////////////////////////////////////////////
#define XSST_LAT_LADDR_OFFSET          0
#define XSST_LAT_LADDR_WIDTH           5

#define XSST_LAT_LVAL_OFFSET           5
#define XSST_LAT_LVAL_WIDTH            1

#define XSST_LAT_INSYS_OFFSET          6
#define XSST_LAT_INSYS_WIDTH           1

#define XSST_LAT_SPLIT_OFFSET          7
#define XSST_LAT_SPLIT_WIDTH           1

#define XSST_LAT_VFEN_OFFSET           8
#define XSST_LAT_VFEN_WIDTH            1

#define XSST_LAT_VPORT_OFFSET          9
#define XSST_LAT_VPORT_WIDTH           3

/////////////////////////////////////////////////////////////////////////////////
///////////////////////////                          ////////////////////////////
///////////////////////////     OVERLAY Field        ////////////////////////////
///////////////////////////                          ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//   This section describes the structure of the overlay vector for the various
//   endpoint variations.
//
////////////////////////////////////////////////////////////////////////////////

//
// Control Endpoint (Null/Pre/Split) offsets within the overlay field
//
#define OVERLAY_BCO_OFFSET              0
#define OVERLAY_BCO_WIDTH               1

#define OVERLAY_BCI_OFFSET              1
#define OVERLAY_BCI_WIDTH               1

#define OVERLAY_INSYSOVRD_OFFSET        2
#define OVERLAY_INSYSOVRD_WIDTH         1

#define OVERLAY_SETUP_RSP_TGL_OFFSET    3
#define OVERLAY_SETUP_RSP_TGL_WIDTH     1

#define OVERLAY_SETUP_RSP_PNDG_OFFSET   4
#define OVERLAY_SETUP_RSP_PNDG_WIDTH    1

#define OVERLAY_CTRL_RSP_STALL_OFFSET   5
#define OVERLAY_CTRL_RSP_STALL_WIDTH    1

#define OVERLAY_CTRL_RSP_NULLD0_OFFSET  6
#define OVERLAY_CTRL_RSP_NULLD0_WIDTH   1

#define OVERLAY_CTRL_RSP_NULLD1_OFFSET  7
#define OVERLAY_CTRL_RSP_NULLD1_WIDTH   1

//
// Bulk/ Non-Accelerated (Null/Pre/Split) offsets within the overlay field
//
#define OVERLAY_IBULK_MSA_EP_PAIR_OFFSET  0
#define OVERLAY_IBULK_MSA_EP_PAIR_WIDTH   4

#define OVERLAY_OBULK_MSA_EP_PAIR_OFFSET  4
#define OVERLAY_OBULK_MSA_EP_PAIR_WIDTH   4

//
// Interrupt (Null/Pre) offsets within the overlay field
//
#define OVERLAY_INT_OPTNAK_FULHLFRTE_OFFSET  0
#define OVERLAY_INT_OPTNAK_FULHLFRTE_WIDTH   1

#define OVERLAY_INT_COPY_TO_CPU_OFFSET    1
#define OVERLAY_INT_COPY_TO_CPU_WIDTH     1

#define OVERLAY_INT_IPERBUFSTATE_OFFSET   2
#define OVERLAY_INT_IPERBUFSTATE_WIDTH    1
#define OVERLAY_INT_IPERBUFLMT_OFFSET     3
#define OVERLAY_INT_IPERBUFLMT_WIDTH      3

//
// Isochronous (Null/Pre/SPLIT) offsets within the overlay field
//
#define OVERLAY_ISO_IPERBUFSTATE_OFFSET   OVERLAY_INT_IPERBUFSTATE_OFFSET
#define OVERLAY_ISO_IPERBUFSTATE_WIDTH    OVERLAY_INT_IPERBUFSTATE_WIDTH
#define OVERLAY_ISO_IPERBUFLMT_OFFSET     OVERLAY_INT_IPERBUFLMT_OFFSET
#define OVERLAY_ISO_IPERBUFLMT_WIDTH      OVERLAY_INT_IPERBUFLMT_WIDTH

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///////////////////////////                          ////////////////////////////
///////////////////////////     Overlay Field        ////////////////////////////
///////////////////////////   to full XSST mapping   ////////////////////////////
///////////////////////////                          ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//   This section describes the structure of the overlay table for the various
//   endpoint variations.  This is mainly to be used by software
//
////////////////////////////////////////////////////////////////////////////////

//
// Control Endpoints (Null/Pre/Split) offsets relative to the XSST
//
#define XSST_BCO_OFFSET                   (XSST_OVRLAY_OFFSET + OVERLAY_BCO_OFFSET)
#define XSST_BCO_WIDTH                    OVERLAY_BCO_WIDTH

#define XSST_BCI_OFFSET                   (XSST_OVRLAY_OFFSET + OVERLAY_BCI_OFFSET)
#define XSST_BCI_WIDTH                    OVERLAY_BCI_WIDTH

#define XSST_INSYSOVRD_OFFSET             (XSST_OVRLAY_OFFSET + OVERLAY_INSYSOVRD_OFFSET)
#define XSST_INSYSOVRD_WIDTH              OVERLAY_INSYSOVRD_WIDTH

#define XSST_SETUP_RSP_TGL_OFFSET         (XSST_OVRLAY_OFFSET + OVERLAY_SETUP_RSP_TGL_OFFSET)
#define XSST_SETUP_RSP_TGL_WIDTH          OVERLAY_SETUP_RSP_TGL_WIDTH

#define XSST_SETUP_RSP_PNDG_OFFSET        (XSST_OVRLAY_OFFSET + OVERLAY_SETUP_RSP_PNDG_OFFSET)
#define XSST_SETUP_RSP_PNDG_WIDTH         OVERLAY_SETUP_RSP_PNDG_WIDTH

#define XSST_CTRL_RSP_STALL_OFFSET        (XSST_OVRLAY_OFFSET + OVERLAY_CTRL_RSP_STALL_OFFSET)
#define XSST_CTRL_RSP_STALL_WIDTH         OVERLAY_CTRL_RSP_STALL_WIDTH

#define XSST_CTRL_RSP_NULLD0_OFFSET       (XSST_OVRLAY_OFFSET + OVERLAY_CTRL_RSP_NULLD0_OFFSET)
#define XSST_CTRL_RSP_NULLD0_WIDTH        OVERLAY_CTRL_RSP_NULLD0_WIDTH

#define XSST_CTRL_RSP_NULLD1_OFFSET       (XSST_OVRLAY_OFFSET + OVERLAY_CTRL_RSP_NULLD1_OFFSET)
#define XSST_CTRL_RSP_NULLD1_WIDTH        OVERLAY_CTRL_RSP_NULLD1_WIDTH

//
// BULK Overlay
//
#define XSST_IMSA_EP_PAIR_OFFSET           (XSST_OVRLAY_OFFSET + OVERLAY_IBULK_MSA_EP_PAIR_OFFSET)
#define XSST_IMSA_EP_PAIR_WIDTH            OVERLAY_IBULK_MSA_EP_PAIR_WIDTH

#define XSST_OMSA_EP_PAIR_OFFSET           (XSST_OVRLAY_OFFSET + OVERLAY_OBULK_MSA_EP_PAIR_OFFSET)
#define XSST_OMSA_EP_PAIR_WIDTH            OVERLAY_OBULK_MSA_EP_PAIR_WIDTH


//
// NEW INT Overlay
//
#define XSST_INT_OPTNAK_FULHLFRTE_OFFSET  (XSST_OVRLAY_OFFSET+OVERLAY_INT_OPTNAK_FULHLFRTE_OFFSET)
#define XSST_INT_OPTNAK_FULHLFRTE_WIDTH   OVERLAY_INT_OPTNAK_FULHLFRTE_WIDTH
#define XSST_INT_COPY_TO_CPU_OFFSET       (XSST_OVRLAY_OFFSET+OVERLAY_INT_COPY_TO_CPU_OFFSET)
#define XSST_INT_COPY_TO_CPU_WIDTH        OVERLAY_INT_COPY_TO_CPU_WIDTH
#define XSST_INT_IPERBUFSTATE_OFFSET      (XSST_OVRLAY_OFFSET+OVERLAY_INT_IPERBUFSTATE_OFFSET)
#define XSST_INT_IPERBUFSTATE_WIDTH       OVERLAY_INT_IPERBUFSTATE_WIDTH
#define XSST_INT_IPERBUFLMT_OFFSET        (XSST_OVRLAY_OFFSET+OVERLAY_INT_IPERBUFLMT_OFFSET)
#define XSST_INT_IPERBUFLMT_WIDTH         OVERLAY_INT_IPERBUFLMT_WIDTH

//
// NEW ISO Overlay
//
#define XSST_ISO_IPERBUFSTATE_OFFSET      (XSST_OVRLAY_OFFSET+OVERLAY_ISO_IPERBUFSTATE_OFFSET)
#define XSST_ISO_IPERBUFSTATE_WIDTH       OVERLAY_ISO_IPERBUFSTATE_WIDTH
#define XSST_ISO_IPERBUFLMT_OFFSET        (XSST_OVRLAY_OFFSET+OVERLAY_ISO_IPERBUFLMT_OFFSET)
#define XSST_ISO_IPERBUFLMT_WIDTH         OVERLAY_ISO_IPERBUFLMT_WIDTH



/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
///////////////////////////                          ////////////////////////////
///////////////////////////        Constants         ////////////////////////////
///////////////////////////                          ////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
//
// Description:
//
//  This section contains constants for different feilds of the XSST
//
//
////////////////////////////////////////////////////////////////////////////////

//
// Valid FWD2CPU definitions
//
#define XSST_FWD2CPU_NORM        0
#define XSST_FWD2CPU_DNS         1
#define XSST_FWD2CPU_DNSUPS_BLK  2
#define XSST_FWD2CPU_XCRM_POST   3






// Script to convert above Verilog to C : sed -r -e 's/#define/\#define/' -e 's///' -e s'///' -e '/___ICRON_FW_END___/q' < xsst.vh > xsst_fw.h
