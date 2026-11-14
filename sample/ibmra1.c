/*
 * ibmra1.c - sample / test program for IBMRA1.SYS
 *            (IBM Rapid Access Keyboard I enabler)
 *
 * 32-bit OS/2 program. Run it after IBMRA1.SYS is loaded:
 *
 *   ibmra1.exe            open \DEV\IBMRA1 and read one byte
 *   ibmra1.exe <value>    write <value> (0..255) first, then read it back
 *
 * The driver sends a write to the keyboard controller as the keyboard LED
 * command, so values 0..7 are the LED bits and other values may not be
 * meaningful. The read returns the byte the driver keeps (the last value
 * written).
 *
 * Build:  wmake -f Makefile.wat        (produces bin\ibmra1.exe)
 *
 * Part of the IBMRA1.SYS project. The original driver code is (C) 2001 Bart van
 * Leeuwen; this sample was written in 2026 and is under the same terms as the
 * rest of the project. See LICENSE.
 */

#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>

#define DEVICE_NAME "\\DEV\\IBMRA1"

int main( int argc, char *argv[] )
{
    HFILE  hf;
    ULONG  action, count;
    UCHAR  value = 0;
    APIRET rc;

    rc = DosOpen( DEVICE_NAME, &hf, &action, 0, FILE_NORMAL,
                  FILE_OPEN,
                  OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE, NULL );
    if( rc != 0 ) {
        printf( "IBMRA1: cannot open %s (DosOpen rc=%lu).\n", DEVICE_NAME, rc );
        printf( "IBMRA1: is IBMRA1.SYS loaded (DEVICE=... in CONFIG.SYS)?\n" );
        return 1;
    }

    if( argc > 1 ) {
        unsigned long v = strtoul( argv[1], NULL, 0 );

        if( v > 255 ) {
            printf( "IBMRA1: value %lu is not a byte (0..255).\n", v );
            DosClose( hf );
            return 1;
        }
        value = (UCHAR)v;
        rc = DosWrite( hf, &value, 1, &count );
        if( rc != 0 || count != 1 ) {
            printf( "IBMRA1: write failed (DosWrite rc=%lu, count=%lu).\n", rc, count );
            DosClose( hf );
            return 1;
        }
        printf( "IBMRA1: wrote 0x%02X (%lu byte).\n", value, count );
    }

    count = 0;
    rc = DosRead( hf, &value, 1, &count );
    if( rc != 0 || count != 1 ) {
        printf( "IBMRA1: read failed (DosRead rc=%lu, count=%lu).\n", rc, count );
        DosClose( hf );
        return 1;
    }
    printf( "IBMRA1: read 0x%02X (%lu byte).\n", value, count );

    rc = DosClose( hf );
    if( rc != 0 ) {
        printf( "IBMRA1: close failed (DosClose rc=%lu).\n", rc );
        return 1;
    }
    return 0;
}
