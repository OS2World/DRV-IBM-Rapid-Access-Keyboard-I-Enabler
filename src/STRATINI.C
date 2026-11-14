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

/*
 * 2026 (ArcaOS port): the boot messages now follow "Plan - Driver Updates.txt":
 * quiet by default (errors only), /V verbose, /Q no messages at all, /W wait
 * for a key, /AA beep when the driver is loaded. Every line starts with
 * "IBMRA1: " and is wrapped at 79 columns. The copyright banner was removed
 * from the boot messages. Keyboard controller waits are now bounded, and the
 * driver does not stay resident if the controller does not answer.
 * See doc\CHANGES.md.
 */

#include "devdefs.h"
#include <i86.h>
#include <conio.h>
#include "devreqp.h"
#include "devaux.h"
#include "version.h"
#include "i8042.h"

// Ensure that the Initialization code located at the end of the driver

#pragma code_seg ( "_INITCODE" ) ;
#pragma data_seg ( "_INITDATA", "INITDATA" ) ;

extern USHORT  OffFinalCS;
extern USHORT  OffFinalDS;

#define PREFIX  "IBMRA1: "

/* options on the DEVICE= line */
static int fVerbose, fQuiet, fWait, fBeep;

/* static work areas: the driver runs with SS != DS, so nothing that a DOS
   call sees may live on the stack */
static USHORT done, got;
static char   key;
static char   line[96];
static char   text[160];

/* ---- small helpers (no C library in a device driver) ---------------------- */
static int Slen( const char *s )
{
    int n = 0;
    while( s[n] )
        n++;
    return n;
}

static int Upc( int c )
{
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

/* Print one message: "IBMRA1: text", wrapped at 79 columns, prefix on every line. */
static void Emit( const char *msg )
{
    int len, n, cut, i, k;
    const int prefix = sizeof( PREFIX ) - 1;
    const int width  = 79 - prefix - 2;

    if( fQuiet )
        return;
    len = Slen( msg );
    while( len > 0 ) {
        n = len;
        if( n > width ) {
            cut = width;
            while( cut > 0 && msg[cut] != ' ' )
                cut--;
            n = cut ? cut : width;
        }
        k = 0;
        for( i = 0; i < prefix; i++ )
            line[k++] = PREFIX[i];
        for( i = 0; i < n; i++ )
            line[k++] = msg[i];
        line[k++] = '\r';
        line[k++] = '\n';
        DosWrite( 1, (void FAR *)line, k, (USHORT FAR *)&done );
        msg += n;
        len -= n;
        if( len > 0 && *msg == ' ' ) {
            msg++;
            len--;
        }
    }
}

/* append the text of a value to text[] */
static void AddNum( unsigned value, int hex )
{
    static const char digits[] = "0123456789ABCDEF";
    char tmp[8];
    int  n = 0, i, base = hex ? 16 : 10;

    do {
        tmp[n++] = digits[value % base];
        value /= base;
    } while( value );
    i = Slen( text );
    if( hex ) {
        text[i++] = '0';
        text[i++] = 'x';
    }
    while( n )
        text[i++] = tmp[--n];
    text[i] = 0;
}

static void AddStr( const char *s )
{
    int i = Slen( text ), j = 0;

    while( s[j] && i < (int)sizeof( text ) - 1 )
        text[i++] = s[j++];
    text[i] = 0;
}

/* DEVICE=...\IBMRA1.SYS [/V] [/Q] [/W] [/AA]
   Unknown options and extra words are ignored, as in the original driver. */
static void ParseArgs( const char FAR *p )
{
    if( p == 0 )
        return;

    while( *p == ' ' || *p == '\t' )
        p++;
    while( *p && *p != ' ' && *p != '\t' )
        p++;                                  /* skip the driver file name */

    for( ;; ) {
        while( *p == ' ' || *p == '\t' )
            p++;
        if( *p == 0 || *p == '\r' || *p == '\n' )
            break;
        if( *p == '/' || *p == '-' ) {
            p++;
            if( Upc( p[0] ) == 'A' && Upc( p[1] ) == 'A' ) {
                fBeep = 1;
                p += 2;
            } else {
                switch( Upc( *p ) ) {
                case 'V':
                    fVerbose = 1;
                    break;
                case 'Q':
                    fQuiet = 1;
                    break;
                case 'W':
                    fWait = 1;
                    break;
                default:
                    break;                    /* unknown options are ignored */
                }
                if( *p )
                    p++;
            }
        } else {
            while( *p && *p != ' ' && *p != '\t' )
                p++;                          /* ignore other arguments */
        }
    }
}

extern VOID StratInit( REQP_INIT FAR *rp )
{
    USHORT        ret;
    unsigned char status;
    int           ok;

    /* save the device helper routine; needed by the DevHlp functions */
    DevHlp = rp->in.devhlp;

    /* the arguments live in the INIT packet, which the output fields overwrite;
       so read them before writing the output fields */
    ParseArgs( (const char FAR *)rp->in.parms );

    /* look for the keyboard controller and send the enable sequence */
    status = (unsigned char)inp( KBD_STATUS_PORT );
    if( status == 0xFF ) {
        ok = 0;                               /* nothing at 0x60/0x64 */
    } else {
        ok = KbdWrite( 0xEA ) && KbdWrite( 0x71 ) && KbdWrite( 0x00 );
    }

    /* save the end of the code and data segments (offsets); everything beyond
       will not exist after this call */
    if( ok ) {
        rp->out.finalcs = FP_OFF( &OffFinalCS );
        rp->out.finalds = FP_OFF( &OffFinalDS );
    } else {
        rp->out.finalcs = 0;
        rp->out.finalds = 0;
    }

    if( ok ) {
        if( fVerbose ) {
            text[0] = 0;
            AddStr( "IBMRA1.SYS version " VER_STRING " (" VER_DATE ")." );
            Emit( text );
            text[0] = 0;
            AddStr( "Keyboard controller present at ports 0x60/0x64 (status " );
            AddNum( status, 1 );
            AddStr( "). Enable sequence sent." );
            Emit( text );
        }
        if( fWait && !fQuiet ) {
            Emit( "Press any key to continue." );
            DosRead( 0, (void FAR *)&key, 1, (USHORT FAR *)&got );
        }
        if( fBeep )
            DosBeep( 1000, 120 );
    } else {
        text[0] = 0;
        AddStr( "IBMRA1.SYS could not be loaded. " );
        if( status == 0xFF ) {
            AddStr( "No keyboard controller was found at ports 0x60 and 0x64 "
                    "(port 0x64 reads 0xFF)." );
        } else {
            AddStr( "The keyboard controller at ports 0x60 and 0x64 did not "
                    "accept the enable sequence." );
        }
        Emit( text );
        if( fWait && !fQuiet ) {
            Emit( "Press any key to continue." );
            DosRead( 0, (void FAR *)&key, 1, (USHORT FAR *)&got );
        }
    }

    if( ok )
        rp->header.status |= RPDONE;
    else
        rp->header.status |= RPDONE | RPERR | RPERR_GENERAL;
}
