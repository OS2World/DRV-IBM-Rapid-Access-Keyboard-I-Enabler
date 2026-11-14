@echo off
rem ---------------------------------------------------------------------------
rem  compile_wat.cmd - build IBMRA1.SYS with Open Watcom 2 (wmake, wcc, wasm, wlink)
rem  Run from the project root:   compile_wat.cmd [clean]
rem  Everything is logged to compile.log (appended). Result: bin\ibmra1.sys
rem ---------------------------------------------------------------------------
if "%1"=="_LOGGED_" goto :run
set PATH=C:\usr\bin;%PATH%
C:\OS2\CMD.EXE /c compile_wat.cmd _LOGGED_ %1 2>&1 | tee -a compile.log
goto :end

:run
set WATCOM=C:\WATCOM
set PATH=C:\WATCOM\BINP;C:\WATCOM\BINW;%PATH%
set INCLUDE=
set EDPATH=C:\WATCOM\EDDAT
echo.
echo ===== ibmra1 build started =====
if "%2"=="clean" goto :clean
wmake -h -f Makefile.wat
if errorlevel 1 goto :failed
if not exist bin\ibmra1.sys goto :failed
dir bin\ibmra1.sys
echo ===== BUILD OK =====
goto :end

:clean
wmake -h -f Makefile.wat clean
echo ===== clean done =====
goto :end

:failed
echo ===== BUILD FAILED =====

:end
