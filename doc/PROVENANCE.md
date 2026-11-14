# Provenance and license check

## Origin
IBMRA1.SYS ("IBM Rapid Access Keyboard I enabler") was written in 2001 by Bart van Leeuwen and released as
version 0.0.1. The original readme (`doc\original\readme.txt`) says:

```
IBM Rapid Access Keyboard I Enabler
v0.0.1 (C) 2001 Bart van Leeuwen <bart@netage.nl>
...
Disclaimer:
Use this software at you own risk.
Its stil a Alpha version !
```

The driver is built on the OS/2 physical device driver sample code that Rick Fishman (Code Blazers, Inc.) published
in the Fall 1991 issue of *IBM Personal Systems Developer* and that WATCOM International Corp. later adapted for
Watcom C/C++. Every source file still starts with that banner; the 2001 author replaced the sample's body with the
keyboard-controller code.

## License: none found
There is no license text in the repository - not in the original commit (638d01b, 2001-01-07), not in the readme and
not in any source file. The only statements are the `(C) 2001 Bart van Leeuwen` line and the "use at your own risk"
disclaimer, which grant no rights to copy, modify or redistribute. The 1991 sample code carries no license text
either.

**Result: the rights are unresolved.** Nothing in this repository grants a license. The 2026 port does not change
that: the new files (`src\version.h`, `src\include\I8042.H`, `src\bldlevel.lnk`, `sample\ibmra1.c`, `test\`, the
build scripts and the documentation) are offered on the same terms as the rest, which is to say without a grant.
`LICENSE` says so and `README.md` repeats it. Do not publish or redistribute the driver or its source until the
rights holder has clarified the license.

## Comparison with the IBM DDK (2026-11-14)
To make sure the 2026 work did not introduce IBM DDK material (the DDK license does not allow redistribution), every
source and text file of the project was compared line by line with the IBM OS/2 DDK 2004 and DDK 1.0 trees on this
machine: 8,764 `.h`/`.inc`/`.c`/`.asm` files, 876,936 distinct lines. Lines were lower-cased, white space was
collapsed, and only lines of 18 characters or more were compared, so that short generic lines do not distort the
result.

| File | Lines shared with the DDK | What they are |
|---|---|---|
| `doc\original\MAKEFILE` | 0 of 69 | - |
| `src\DEVSEGS.ASM` | 8 of 23 (34.8 %) | `const segment word public use16 'data'` and similar segment declarations - the only way to write them in MASM syntax |
| `src\HEADER.C` | 2 of 20 (10.0 %) | `#include "devhdr.h"`, `extern void strategy();` |
| `src\STRATEGY.C` | 1 of 65 (1.5 %) | `#include <conio.h>` |
| `src\STRATINI.C` | 3 of 118 (2.5 %) | `#include <conio.h>`, `unsigned char status;` and one more trivial line |
| `src\include\DEVDEFS.H` | 3 of 3 (100 %) | the three lines of the file: `#include <os2def.h>`, `#include <bsedos.h>`, `typedef void near * npvoid;` |
| `src\include\DEVHDR.H` | 2 of 38 (5.3 %) | `#ifdef __cplusplus` |
| `src\include\DEVREQP.H` | 2 of 74 (2.7 %) | `#ifdef __cplusplus` |
| `src\include\DEVAUX.H` | 0 of 17 | - |
| `src\include\HRDEV.H` | 0 of 6 | - |
| `src\include\HRTIMER.H` | 0 of 2 | - |

No file of the project is a copy of a DDK file, and no DDK file is included in the tree. The matching lines are
includes, include guards, structure-less segment declarations and one generic typedef - expressions that any driver
of this kind contains. The comments and the request-packet definitions in `src\include\` are this project's own
text; they were written for the 2026 port from the published OS/2 device driver interface, not copied from
`reqpkt.h`/`devhlp.h`.

Not covered by the comparison: the DDK PDF and HTML books (not source code), and the 1991 magazine article itself
(not on this machine).

## Action taken
* Nothing had to be removed for the DDK check: the 2026 changes add only new code and text.
* The new files must stay free of DDK code: do not paste `reqpkt.h`, `devhlp.h`, `dhcalls.h` or any DDK sample into
  this project. `src\include\DEVAUX.H` shows the alternative: the few DevHlp calls the driver needs are inline
  assembly, written from the documented interface.
* The license question is documented in `LICENSE` and `README.md`; the repository must not be made public until it
  is resolved.

## Open points
* The 1991 sample code: the rights belong to its author and/or IBM. The 2001 driver already shipped without a
  license, so the question is older than this port.
* Who to ask: Bart van Leeuwen (the 2001 address was `bart@netage.nl`; it may no longer work) and OS2World, who
  published the original repository. If neither can be reached, the conservative choice is to keep the code
  private, or to publish only the documentation and the patch, not the original files.
* The 1991 article's own terms ("may be freely used" or similar) could not be checked. If a copy of the Fall 1991
  issue turns up, its statement should be added here.
