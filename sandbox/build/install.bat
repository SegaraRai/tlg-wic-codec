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

MKDIR x86 2> nul
MKDIR x64 2> nul

XCOPY /Y tlg-wic-codec-x86.dll x86\tlg-wic-codec.dll
XCOPY /Y tlg-wic-codec-x64.dll x64\tlg-wic-codec.dll

regsvr32 /s x86\tlg-wic-codec.dll
regsvr32 /s x64\tlg-wic-codec.dll

REG IMPORT install.reg
