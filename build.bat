@echo off

SET CC=gcc
SET CFLAGS=
SET SRC=main.c

%CC% %CFLAGS% %SRC% -o swt.exe
