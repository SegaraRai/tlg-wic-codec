@echo off

bash -O globstar -O nullglob -c "clang-format.exe -i **/*.c **/*.cpp **/*.h **/*.hpp"
