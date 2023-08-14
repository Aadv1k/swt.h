@echo off

set CC=gcc
set CFLAGS=-Wall -Wextra -pedantic

if "%1" == "DEBUG" (
  set CFLAGS=%CFLAGS% -ggdb
)

set SRC=main.c

%CC% %CFLAGS% %SRC% -o swt.exe
