@echo off

SET CC=gcc
SET CFLAGS=-Wall -Wextra
SET SRC=main.c

%CC% %CFLAGS% %SRC% -o swt.exe
