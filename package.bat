@ECHO OFF

CD "%~dp0"

DEL tlg-wic-codec.zip

RMDIR /S /Q package

MKDIR package

CD package

MKDIR x86
XCOPY ..\Release-Win32\tlg-wic-codec.dll x86

MKDIR x64
XCOPY ..\Release-x64\tlg-wic-codec.dll x64

XCOPY ..\*.reg .

XCOPY ..\README.md .
XCOPY ..\LICENSE .
XCOPY ..\THIRDPARTY .

CD ..

"C:\Program Files\7-Zip\7z.exe" a -y -mx9 tlg-wic-codec.zip .\package\*

RMDIR /S /Q package
