@echo off
rem smoke.cmd - quick test of IBMRA1.SYS
rem Run from the project root after the driver is loaded and bin\ibmra1.exe is built.
rem It opens \DEV\IBMRA1 and reads one byte; it does not change the keyboard LEDs.
bin\ibmra1.exe
if errorlevel 1 goto :failed
echo smoke test OK
goto :end
:failed
echo smoke test FAILED
:end
