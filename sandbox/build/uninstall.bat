@ECHO OFF

IF NOT "%NUMBER_OF_PROCESSORS%"=="4" (
  ECHO This script should be run on Hyper-V virtual machine.
  EXIT /B
)

NET SESSION > NUL 2>&1
IF NOT %ERRORLEVEL% == 0 (
  ECHO This script should be run as administrator.
  EXIT /B
)

CD %~dp0

REG IMPORT uninstall.reg

regsvr32 /u x86\tlg-wic-codec.dll
regsvr32 /u x64\tlg-wic-codec.dll
