@echo off

set CC=gcc
set CFLAGS=-Wall -Wextra -pedantic
set SRCS=main.c
set OUT=swt.exe

if "%1" == "DEBUG" (
  set CFLAGS=%CFLAGS% -ggdb
  set OUT=swt_debug.exe
)

if "%1" == "TEST" (
    set SRCS=./tests/*.c ./thirdparty/munit.c
    set OUT=swt_test.exe
)



%CC% %CFLAGS% %SRCS% -o %OUT%
