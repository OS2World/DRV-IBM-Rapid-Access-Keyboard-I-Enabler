# Changes from the original (upstream-original, version 0.0.1, 2001-01-07)

Original: https://github.com/OS2World/DRV-IBM-Rapid-Access-Keyboard-I-Enabler (tag `upstream-original`)

Goal: build the driver with Open Watcom 2 on ArcaOS, bring it to the common project layout and follow the boot
message rules of `Plan - Driver Updates.txt`. The work is split in the plan's order: (1) build with the new tools,
no logic change; (2) boot messages and parameters, including the fixes needed for that.

## Version
* 0.0.1 -> **0.0.2** (`src\version.h`, `src\bldlevel.lnk`). The original boot banner called itself "Version 1.0",
  which did not match the published 0.0.1. The new banner prints 0.0.2 and the release date.

## Project layout (no code change)
* `HEADER.C`, `STRATEGY.C`, `STRATINI.C`, `DEVSEGS.ASM` moved to `src\`.
* `include\` moved to `src\include\`.
* `MAKEFILE` and `readme.txt` moved to `doc\original\` (kept for reference, not used by the new build).
* Added `Makefile.wat`, `compile_wat.cmd`, `README.md`, `LICENSE`, `doc\CHANGES.md`, `doc\PROVENANCE.md`,
  `sample\`, `test\`, `.gitignore`, `.gitattributes`. `bin\` is build output and is not in git.

## Step 1: build with Open Watcom 2 (no logic change)
### `Makefile.wat` (new)
* 16-bit C with `wcc`, flags taken from the original makefile: `-bt=os2 -ms -5 -s -zdp -zff -zgf -zu -zl -zp1 -zq
  -d2 -d_PDD_`, include path `src\include`, `$(%WATCOM)\h`, `$(%WATCOM)\h\os21x`.
* Assembler `wasm -bt=os2 -d1 -zq`.
* Object order `devsegs, strategy, stratini, header`: `DEVSEGS.obj` first so that it declares the segment order
  and the device header (`_HEADER`) is at offset 0 of `DGROUP`.
* Link recipe from the proven `DRV-MISC-RNG-810` / `DEV-SAMPLE-DRV-wedge` drivers:
  `system os2 dll initglobal`, `option protmode`, explicit `os2.lib` and `clibs.lib`. The exact link file is in
  README.md ("Link recipe"). The original link step (`wlink` options) was not in the repository.
* `wcc386` + `wlink system os2v2` build the 32-bit sample `bin\ibmra1.exe`.

### `compile_wat.cmd` (new)
* Sets `WATCOM=C:\WATCOM`, `PATH`, empty `INCLUDE` and `EDPATH` itself; runs `wmake -h -f Makefile.wat`.
* Logs everything with `tee` to `compile.log` and ends with `===== BUILD OK: bin\ibmra1.sys =====` (after the
  `dir` output with the name and size) or `===== BUILD FAILED... =====`.
* `compile_wat.cmd clean` removes `bin\` (wmake clean) and the log. CRLF line endings.

### `src\DEVSEGS.ASM`
* One line: `assume cs:CGROUP` -> `assume cs:_TEXT`. `wasm` does not define a symbol called `CGROUP`; the
  original include file that defined it is not in the repository. The segment order and `DGROUP` are unchanged.
  This is the only source change needed to build the original code. The assembler is `wasm` because the file is
  tiny MASM syntax and rewriting it for NASM would be a larger change (plan section 1).

### Warnings
The baseline build warned about a missing prototype and pointer type mismatches in `STRATEGY.C`, and about the
`-1` initializer for the device header's next pointer in `HEADER.C`. The `STRATEGY.C` warnings were removed in
step 2 (see below). `HEADER.C` is unchanged and still gives W101 for the original `-1` initializer; it is
harmless (the loader replaces the field) and was left as it was.

### Baseline
`bin\ibmra1.sys` built from the unchanged original: **1256 bytes**, SHA256
`C6F9880F1B87CF6FEF7185D228CE384CAAC8F174C3FA812E710434095EB722F7` (saved as `bin\ibmra1-baseline.sys`).
NE structure checked: device header at `DGROUP:0000`, next `0xFFFFFFFF`, attributes `0x8880`, name `IBMRA1  `;
imports only `DOSWRITE`.

## Step 2: boot messages, parameters, robustness
### `src\STRATINI.C` (rewritten INIT part, logic otherwise unchanged)
* Removed the copyright banner ("IBM Rapid Access Keyboard I enabler - Version 1.0 / Bart van Leeuwen ...") from
  the boot messages, as section 5 requires. Copyright and author information is in README.md, LICENSE and the
  source headers.
* Quiet by default: nothing on success. Options `/V` (verbose), `/Q` (no messages at all), `/W` (wait for a key),
  `/AA` (beep). Unknown options and extra words are ignored, as in the original driver.
* Every line is printed as `IBMRA1: <text>`, word-wrapped at 79 columns, with the prefix repeated on every line.
* `/V` prints the version and build date (`VER_STRING`/`VER_DATE` from `src\version.h`) and the keyboard
  controller status read from port 0x64.
* Error text: `IBMRA1: IBMRA1.SYS could not be loaded. <reason>` (no controller at 0x60/0x64, or the controller
  did not accept the enable sequence). The driver then sets `finalcs`/`finalds` to 0 and returns an error, so it
  does not stay resident.
* Behaviour change: the original always loaded and wrote the enable sequence even when no controller answered.
  Now a missing/stuck controller is reported and the driver does not load. On a machine where the extra keys
  cannot work this is the correct result; the message tells the user why.
* The three controller waits (`inp(0x64)` loops) are bounded: if the controller does not clear the input-buffer
  bit, the driver gives up instead of hanging the boot. The bytes sent (`0xEA`, `0x71`, `0x00`) are unchanged.
* The INIT packet argument pointer is read before the output fields (`finalcs`/`finalds`) overwrite it. The
  structure layout in `src\include\DEVREQP.H` was checked against the DDK `reqpkt.h` and is correct (far pointer
  at offset 18); no change was needed.

### `src\STRATEGY.C`
* `#include "i8042.h"`; the unbounded waits in `StratWrite` were replaced by `KbdWrite()` (bounded). The write
  path also waits before the first byte (`0xEB`); the original wrote it without a wait. Bytes and order are
  otherwise unchanged.
* Added the prototype for `StratInit()` and explicit request-packet casts in the dispatch switch, which removes
  the baseline W131/W1176 warnings. No logic change.

### `src\include\I8042.H` (new)
* `KbdWaitWrite()` / `KbdWrite()`: bounded polling of status port 0x64, data port 0x60. Used by INIT and by the
  write path. Keeps the driver from hanging with interrupts disabled.

### `src\version.h` (new)
* `VER_STRING "0.0.2"`, `VER_DATE "14 Nov 2026"`, `DEVICE_NAME "IBMRA1"`; keep `src\bldlevel.lnk` in sync.

### `src\bldlevel.lnk` (new)
* BLDLEVEL signature `@#OS2World:0.0.2#@##1## 14 Nov 2026 00:00:00     ArcaOS-Dev::::0::@@IBM Rapid Access
  Keyboard I enabler`, linked with `@src\bldlevel.lnk`. Verified with `BLDLEVEL.EXE`: vendor `OS2World`,
  revision `0.00.2`, machine `ArcaOS-Dev`, description as above.

### `Makefile.wat`
* `src\version.h`, `src\include\I8042.H` added to the dependencies; `@src\bldlevel.lnk` added to the link file;
  `clean` also removes the `bin` directory and `*.err` files.

### `sample\ibmra1.c` (new)
* 32-bit OS/2 program: opens `\DEV\IBMRA1`, reads one byte; with an argument it writes that byte first (the
  driver sends it to the keyboard as the LED command) and reads it back. Built by `Makefile.wat`.

### `test\config.sys.sample`, `test\smoke.cmd` (new)
* CONFIG.SYS excerpt and a read-only smoke test of the device.

### Docs
* `README.md` rewritten to the plan's template; `LICENSE` states that the license is unresolved; this file; and
  `doc\PROVENANCE.md` (origin and DDK comparison).

## Verification of version 0.0.2
* Build: `compile_wat.cmd` -> `===== BUILD OK: bin\ibmra1.sys =====`, no compiler or linker warnings from the
  changed files (only the original W101 from `HEADER.C` remains).
* `bin\ibmra1.sys`: **3446 bytes**, SHA256
  `D3E8BD246AB2D2505BA09F75961DE53E9B891050AE8B2C759DFFD06C4D01432B`.
* Map: `_HEADER` 0x001E at DGROUP:0000, `_DATA` 2, `_BSS` 4, `_INITDATA` 0x0120, `_TEXT` 0x0385,
  `_INITCODE` 0x05D4. Device header: next `0xFFFFFFFF`, attributes `0x8880`, strategy `0x0228`, name `IBMRA1  `.
* BLDLEVEL as above. `bin\ibmra1.exe` (sample) 13982 bytes.
* Boot/load test on the ArcaOS 5.1.1 Dev VM: (filled in after the reboot test).
