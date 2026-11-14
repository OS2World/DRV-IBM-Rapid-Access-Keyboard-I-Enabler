# Makefile.wat - Open Watcom 2.0 build file for IBMRA1.SYS
# Run from the project root:  wmake -f Makefile.wat   (or use compile_wat.cmd)
#
# Tools: wcc (16-bit C), wasm (Open Watcom assembler), wlink.
# Output: bin\ibmra1.sys (16-bit NE character device driver) and bin\ibmra1.map
#
# The original build (doc\original\MAKEFILE) was an Open Watcom 10/11 makefile
# for an OS/2-hosted toolchain. The flags below are the original ones, kept so
# the first build is a baseline of the unmodified source.

SRC  = src
INC  = $(SRC)\include
OBJ  = bin
NAME = ibmra1

# 16-bit C: small model, Pentium, no stack checks, SS != DS (driver stack),
# packed structures, no default-library record, debug info, quiet.
CC      = wcc
CFLAGS  = -bt=os2 -ms -5 -s -zdp -zff -zgf -zu -zl -zp1 -zq -d2 -d_PDD_ &
          -i=$(INC) -i=$(%WATCOM)\h -i=$(%WATCOM)\h\os21x

AS      = wasm
ASFLAGS = -bt=os2 -d1 -zq

# DEVSEGS first: it declares the segment order and the DGROUP layout, so the
# device header (_HEADER) is at offset 0 of the data segment. Keep this order.
OBJS = $(OBJ)\devsegs.obj $(OBJ)\strategy.obj $(OBJ)\stratini.obj $(OBJ)\header.obj
HDRS = $(INC)\DEVDEFS.H $(INC)\DEVHDR.H $(INC)\DEVREQP.H $(INC)\DEVAUX.H

all: $(OBJ)\$(NAME).sys .SYMBOLIC

$(OBJ)\devsegs.obj: $(SRC)\DEVSEGS.ASM
	-@mkdir $(OBJ) > nul 2>&1
	$(AS) $(ASFLAGS) -fo=$@ $(SRC)\DEVSEGS.ASM

$(OBJ)\strategy.obj: $(SRC)\STRATEGY.C $(HDRS)
	-@mkdir $(OBJ) > nul 2>&1
	$(CC) $(CFLAGS) -fo=$@ $(SRC)\STRATEGY.C

$(OBJ)\stratini.obj: $(SRC)\STRATINI.C $(HDRS)
	-@mkdir $(OBJ) > nul 2>&1
	$(CC) $(CFLAGS) -fo=$@ $(SRC)\STRATINI.C

$(OBJ)\header.obj: $(SRC)\HEADER.C $(HDRS)
	-@mkdir $(OBJ) > nul 2>&1
	$(CC) $(CFLAGS) -fo=$@ $(SRC)\HEADER.C

# Link as a 16-bit NE device driver. "system os2 dll initglobal" is the link
# form measured to install from DEVICE= (see README.md, "Link recipe").
$(OBJ)\$(NAME).sys: $(OBJS)
	@%create $(OBJ)\$(NAME).lnk
	@%append $(OBJ)\$(NAME).lnk system os2 dll initglobal
	@%append $(OBJ)\$(NAME).lnk option protmode
	@%append $(OBJ)\$(NAME).lnk name $(OBJ)\$(NAME).sys
	@%append $(OBJ)\$(NAME).lnk option map=$(OBJ)\$(NAME).map
	@%append $(OBJ)\$(NAME).lnk option quiet
	@%append $(OBJ)\$(NAME).lnk library $(%WATCOM)\lib286\os2\os2.lib
	@%append $(OBJ)\$(NAME).lnk library $(%WATCOM)\lib286\os2\clibs.lib
	@%append $(OBJ)\$(NAME).lnk file $(OBJ)\devsegs.obj
	@%append $(OBJ)\$(NAME).lnk file $(OBJ)\strategy.obj
	@%append $(OBJ)\$(NAME).lnk file $(OBJ)\stratini.obj
	@%append $(OBJ)\$(NAME).lnk file $(OBJ)\header.obj
	wlink @$(OBJ)\$(NAME).lnk

clean: .SYMBOLIC
	-@echo y | del $(OBJ)\*.* > nul 2>&1
