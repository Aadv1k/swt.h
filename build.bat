@echo off

SET CC=gcc
SET CFLAGS=-Wall -Wextra -pedantic
SET SRC=main.c

%CC% %CFLAGS% %SRC% -o swt.exe
