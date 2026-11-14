/*
 * I8042.H - bounded wait for the PC keyboard controller (ports 0x60/0x64).
 *
 * The original driver waited for the controller in a loop with no limit:
 *
 *     do { z = inp(0x64); } while ((z & 2) == 2);
 *
 * If the controller is missing or stuck, that loop never ends and the machine
 * hangs (during INIT, or inside a READ/WRITE request with interrupts off).
 * These helpers poll the same status bit but give up after a fixed number of
 * reads. The sequence of I/O writes is unchanged.
 *
 * Part of IBMRA1.SYS. See LICENSE.
 */
#ifndef I8042_H
#define I8042_H

#include <conio.h>

#define KBD_STATUS_PORT  0x64      /* 8042 status register (read)   */
#define KBD_DATA_PORT    0x60      /* 8042 data register (read/write) */

/* Wait until the controller input buffer is empty (status bit 1 clear).
   Returns 1 on success, 0 if the controller did not answer in time. */
static int KbdWaitWrite( void )
{
    unsigned tries = 0x8000;

    while( (inp( KBD_STATUS_PORT ) & 0x02) != 0 && --tries != 0 )
        ;
    return tries != 0;
}

/* Wait for the controller and write one byte to it.
   Returns 1 on success, 0 on timeout. */
static int KbdWrite( unsigned char value )
{
    if( !KbdWaitWrite() )
        return 0;
    outp( KBD_DATA_PORT, value );
    return 1;
}

#endif
