# IBMRA1.SYS - IBM Rapid Access Keyboard I enabler

**Version 0.0.2** (original 0.0.1, 2001)

OS/2 physical device driver that enables the extra buttons on the IBM Rapid Access Keyboard I, the keyboard
supplied with many IBM Aptiva systems. When the driver is loaded, the extra keys send ordinary scan codes, so a
program such as XWorkplace can attach actions to them.

## Status
Builds clean with Open Watcom 2. Loads on the ArcaOS Dev VM (see "Tested on"); not tested on real hardware yet.
The extra LEDs of the keyboard are not supported.

## Tested on
* Build: ArcaOS 5.1.1 Dev VM (VirtualBox), Open Watcom 2.0.1 (`wcc`, `wcc386`, `wasm`, `wlink`, `wmake`),
  2026-11-14.
* Run: (filled in after the boot test on the Dev VM).

## License
3Clauses BSD License

## Authors
* Original driver: Bart van Leeuwen, 2001.
* Base: OS/2 physical device driver sample code by Rick Fishman (Code Blazers, Inc.), published in the Fall 1991
  issue of IBM Personal Systems Developer; adapted for Watcom C/C++ by WATCOM International Corp.
* 2026 Open Watcom 2 / ArcaOS port, sample and documentation: Martin Iturbide.

## Original source
* https://github.com/OS2World/DRV-IBM-Rapid-Access-Keyboard-I-Enabler
* Version 0.0.1, 2001-01-07. The original readme is kept in `doc\original\readme.txt`, the original makefile in
  `doc\original\MAKEFILE`.

## Build tools
Open Watcom 2.0.1 (`wcc` 16-bit C, `wasm` assembler, `wlink`, `wmake`) on ArcaOS. The assembler is Open Watcom's
`wasm`: the only assembly file (`src\DEVSEGS.ASM`) is MASM syntax and needed a one-line change, so rewriting it
for NASM was not worth it.

Build from the project root:

```
compile_wat.cmd            build: bin\ibmra1.sys and bin\ibmra1.exe (sample), log in compile.log
compile_wat.cmd clean      remove bin\ and compile.log
```

`compile_wat.cmd` sets `WATCOM`, `PATH`, `INCLUDE` and `EDPATH` itself; nothing has to be prepared in the
environment. The build ends with `===== BUILD OK: bin\ibmra1.sys =====` or `===== BUILD FAILED... =====`.

Baseline comparison: the unmodified original source builds to 1256 bytes (`bin\ibmra1-baseline.sys`, SHA256
`C6F9880F1B87CF6FEF7185D228CE384CAAC8F174C3FA812E710434095EB722F7`). Version 0.0.2 is 3446 bytes, SHA256
`D3E8BD246AB2D2505BA09F75961DE53E9B891050AE8B2C759DFFD06C4D01432B`; the growth is the BLDLEVEL signature, the
message text and the new INIT code.

## Link recipe
The driver is a 16-bit OS/2 NE module. The exact `wlink` options used by `Makefile.wat` (the recipe proven by the
`DRV-MISC-RNG-810` and `DEV-SAMPLE-DRV-wedge` drivers) are:

```
system os2 dll initglobal
option protmode
name bin\ibmra1.sys
@src\bldlevel.lnk
option map=bin\ibmra1.map
option quiet
library C:\WATCOM\lib286\os2\os2.lib
library C:\WATCOM\lib286\os2\clibs.lib
file bin\devsegs.obj
file bin\strategy.obj
file bin\stratini.obj
file bin\header.obj
```

`bin\devsegs.obj` must be linked first: it declares the segment order and `DGROUP`, and `HEADER.C` puts the device
header (`_HEADER`) at offset 0 of `DGROUP`. Without that order the loader cannot find the device header.
`initglobal` gives the INIT code its own discardable segment (`_INITCODE`). `os2.lib` supplies the DOS calls used
during INIT (`DosWrite`, `DosRead`, `DosBeep`, imported from `DOSCALLS`); `clibs.lib` supplies `inp`/`outp`.
No DH calls library is needed: the DevHlp helpers in `src\include\DEVAUX.H` are inline assembly.

## Install
1. Copy `bin\ibmra1.sys` to `C:\OS2\BOOT\`.
2. Add one line to `CONFIG.SYS`:

```
DEVICE=C:\OS2\BOOT\IBMRA1.SYS
```

Put it after the keyboard driver (`BASEDEV=IBMKBD.SYS` on ArcaOS). This is a character device driver, so it uses
`DEVICE=`, not `BASEDEV=`. A sample is in `test\config.sys.sample`.

Before installing a new build, back up `CONFIG.SYS` and test under a different file name (for example
`IBMRA1.NEW`) so that the old file and the old `CONFIG.SYS` can be restored if the new driver does not load.

If the keyboard controller cannot be found, the driver prints an error and does not stay loaded.

## Parameters
The driver is quiet by default: nothing is printed when it loads correctly.

| Option | Meaning |
|---|---|
| *(none)* | quiet: only errors are shown |
| `/V` | verbose: version, build date, keyboard controller status |
| `/Q` | no messages at all, not even errors |
| `/W` | wait for a key press after the messages |
| `/AA` | audible alert (beep) when the driver is loaded |

Every line starts with `IBMRA1: `; long lines are word-wrapped at 79 columns and every line gets the prefix.
Error text has the form `IBMRA1: IBMRA1.SYS could not be loaded. <reason>`. No copyright, license or author text
is printed at boot.

Example:

```
DEVICE=C:\OS2\BOOT\IBMRA1.SYS /V /W /AA
```

## Sample / test
* `sample\ibmra1.c` is a 32-bit OS/2 program, built to `bin\ibmra1.exe`. Without arguments it opens
  `\DEV\IBMRA1` and reads one byte; with an argument `ibmra1.exe 7` it first writes that byte (the driver sends
  it to the keyboard as the LED command) and then reads it back.
* `test\smoke.cmd` runs `bin\ibmra1.exe` without arguments (read only) and reports OK or FAILED.
* `test\config.sys.sample` is a CONFIG.SYS excerpt with the options.

Run the smoke test from the project root after the driver is loaded:

```
test\smoke.cmd
```

## Known issues
* Not tested on real hardware; tested only on the VirtualBox Dev VM.
* The extra LEDs of the keyboard are not supported (same as the original).
* The driver talks to the keyboard controller at I/O ports 0x60/0x64 directly. Do not load it together with
  another driver that owns the controller.
* The original INIT code waited for the controller in a loop with no limit; version 0.0.2 gives up after a fixed
  number of reads instead of hanging the boot.

## Links
* Original repository: https://github.com/OS2World/DRV-IBM-Rapid-Access-Keyboard-I-Enabler
* EDM2, "Suggestion for OS/2 and eComStation Drivers Arguments": https://www.edm2.com/index.php/Driver_Arguments
* XWorkplace (can attach actions to the extra keys): https://www.xworkplace.org/

## Changelog
### 0.0.2 (2026-11-14)
* Built with Open Watcom 2.0.1 (wcc, wasm, wlink, wmake); new project layout and `compile_wat.cmd`.
* Boot messages and options `/V /Q /W /AA` as in the common driver plan; `IBMRA1: ` prefix, 79-column wrap,
  error text, no copyright banner.
* Keyboard controller waits are bounded; the driver refuses to load if the controller does not answer.
* BLDLEVEL signature (`OS2World:0.0.2`, verified with `BLDLEVEL.EXE`), `src\version.h`.
* Sample program and smoke test added.

### 0.0.1 (2001-01-07)
* Original release by Bart van Leeuwen.
