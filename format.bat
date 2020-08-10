@ECHO OFF

CD "%~dp0"
CD tlg-wic-codec

bash -O globstar -O nullglob -c "clang-format.exe -i **/*.c **/*.cpp **/*.h **/*.hpp"
