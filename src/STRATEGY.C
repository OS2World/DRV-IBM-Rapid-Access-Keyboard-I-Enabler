/*================================================================\
|                                                                 |
|      OS/2 Physical Device Driver Example Code                   |
|                  for Watcom C/C++ 11.0c                         |
|                                                                 |
|  STRATEGY.C (Resident portion of driver)                        |
|                                                                 |
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


ULONG  DevHlp;  // DevHelp Interface Address
CHAR cStatusLed='T';


static VOID StratNoOp( REQP_HEADER FAR *rp )
{
    rp->status = RPDONE;
}

static VOID StratRead( REQP_RWV FAR *rp )
{
        PVOID virt;

       _disable();

        if( DevPhysToVirt( rp->transaddr, rp->count, &virt ) != 0 ){
           // ERROR
            rp->count = 0;
            goto Exit;
        }
        rp->count = 1;
        *(char FAR*)virt = cStatusLed;
Exit:
        _enable();
        rp->header.status |=  RPDONE;

}

static VOID StratWrite( REQP_RWV FAR *rp )
{
        PVOID virt;
        char z;

       _disable();

        if( DevPhysToVirt( rp->transaddr, rp->count, &virt ) != 0 ){
           // ERROR
            rp->count = 0;
            goto Exit;
        }
        rp->count = 1;
        // bvl: it makes no sense to accept values with bit 8 set
        //      this also can cause coruption of the keyb controller
        if(*(char FAR*)virt > 127 )
         cStatusLed = *(char FAR*)virt ^ 128;
        else
         cStatusLed = *(char FAR*)virt;

        outp(0x60,0xeb);
        do {
         z =inp(0x64);
        } while ((z & 2) == 2 );
        outp(0x60,0x00);
        do {
         z =inp(0x64);
        } while ((z & 2) == 2 );
        outp(0x60,cStatusLed);
        do {
         z =inp(0x64);
        } while ((z & 2) == 2 );
Exit:
        _enable();
        rp->header.status |=  RPDONE;

}

static VOID StratOpen( REQP_OPENCLOSE FAR *rp )
{
        rp->header.status |=  RPDONE;
}

static VOID StratClose( REQP_OPENCLOSE FAR *rp )
{
        rp->header.status |=  RPDONE;
}

// Strategy entry point
//
// The strategy entry point must be declared according to the STRATEGY
// calling convention, which fetches arguments from the correct registers.


#pragma aux STRATEGY far parm [es bx];
#pragma aux (STRATEGY) Strategy;

VOID Strategy(REQP_ANY FAR* rp)
{

// Strategy routine for device set in header.c

    if( rp->header.command < RP_END ){
        switch( rp->header.command ){
        case RP_INIT:
            StratInit( rp );
            break;
        case RP_READ:
            StratRead( rp );
            break;
        case RP_READ_NO_WAIT:
        case RP_INPUT_STATUS:
        case RP_INPUT_FLUSH:
        case RP_WRITE:
            StratWrite( rp );
            break;
        case RP_WRITE_VERIFY:
        case RP_OUTPUT_STATUS:
        case RP_OUTPUT_FLUSH:
            StratNoOp( rp );
            break;
        case RP_OPEN:
            StratOpen( rp );
            break;
        case RP_CLOSE:
            StratClose( rp );
            break;
        default:
            rp->header.status = RPERR_COMMAND | RPDONE;
        }
    }else{
        rp->header.status = RPERR_COMMAND | RPDONE;
    }
    return;
}
