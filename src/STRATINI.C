/*================================================================\
|                                                                 |
|      OS/2 Physical Device Driver Example Code                   |
|                  for Watcom C/C++ 11.0c                         |
|                                                                 |
|  STRATINI.C (Discardable portion of driver)                     |
|                                                                 |
|  This device driver provides a high-resolution timer for OS/2   |
|                                                                 |
|  The source code was obtained from the Fall 1991 issue of       |
|  IBM Personal Systems Developer magazine.                       |
|                                                                 |
|                                                                 |
|  Adapted for Watcom C/C++ 10.x by WATCOM International Corp.    |
|  Original Author: Rick Fishman                                  |
|                   Code Blazers, Inc.                            |
|                   4113 Apricot                                  |
|                   Irvine, CA 92720                              |
|                                                                 |
\================================================================*/

#include "devdefs.h"
#include <i86.h>
#include <conio.h>
#include "devreqp.h"
#include "devaux.h"

// Ensure that the Initialization code located at the end of the driver

#pragma code_seg ( "_INITCODE" ) ;
#pragma data_seg ( "_INITDATA", "INITDATA" ) ;

extern USHORT  OffFinalCS;
extern USHORT  OffFinalDS;
#define STRV( a )  a, sizeof( a )   // make  vector of an array

static char CopyRightMsg[] = {
    "\r\nIBM Rapid Access Keyboard I enabler - Version 1.0\r\n"
    "Bart van Leeuwen - bart@netlabs.org\r\n"
    "(c) 29 December 2001\r\n\n"
};

extern VOID StratInit( REQP_INIT FAR *rp )
{
    USHORT ret;
    char z;

    do {
     z =inp(0x64);
    } while ((z & 2) == 2 );  /* wait */

    outp(0x60,0xEA);
    do {
     z =inp(0x64);
    } while ((z & 2) == 2 );
    outp(0x60,0x71);
    do {
     z =inp(0x64);
    } while ((z & 2) == 2 );
    outp(0x60,0x00);

    DosWrite( 1, STRV( CopyRightMsg ), &ret );
    DevHlp = rp->in.devhlp;

    {
        rp->out.finalcs = FP_OFF( &OffFinalCS );
        rp->out.finalds = FP_OFF( &OffFinalDS );
    }
    rp->header.status |=  RPDONE;
}
