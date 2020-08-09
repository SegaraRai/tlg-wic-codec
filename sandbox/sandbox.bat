@ECHO OFF

CD %~dp0

COPY ..\Release-Win32\tlg-wic-codec.dll build\tlg-wic-codec-x86.dll
COPY ..\Release-x64\tlg-wic-codec.dll build\tlg-wic-codec-x64.dll
XCOPY /Y ..\*.reg build\
