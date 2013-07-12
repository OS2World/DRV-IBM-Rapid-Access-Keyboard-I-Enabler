#define STRV( a )  a, sizeof( a )   // make  vector of an array

/*
 * Constants for i8253 timer
 */
enum {
    i8253CountRegister	  = 0x40,   // 8253 Counter Register
    i8253CtrlByteRegister = 0x43,   // 8253 Control Byte Register
    i8253CmdReadCtrZero   = 0x00,   // Latch Command, LSB first, MSB second
    i8253CmdInitCtrZero   = 0x34,   // Rate generator
};
