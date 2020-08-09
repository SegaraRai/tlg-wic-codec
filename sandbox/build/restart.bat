@ECHO OFF

IF NOT "%NUMBER_OF_PROCESSORS%"=="4" (
  ECHO This script should be run on Hyper-V virtual machine.
  EXIT /B
)

CD %~dp0

TASKKILL /F /IM explorer.exe
TASKKILL /F /IM dllhost.exe

DEL %LOCALAPPDATA%\Microsoft\Windows\Explorer\*.db

COPY /Y tlg-wic-codec-x86.dll x86\tlg-wic-codec.dll
COPY /Y tlg-wic-codec-x64.dll x64\tlg-wic-codec.dll

START "" explorer
START "" explorer "%~dp0\images"
