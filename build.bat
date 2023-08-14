@echo off

SET CC=gcc
SET CFLAGS=-Wall -Wextra -pedantic -ggdb -O2
SET SRC=main.c

%CC% %CFLAGS% %SRC% -o swt.exe
