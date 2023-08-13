@echo off

SET CC=gcc
SET CFLAGS=-Wall -Wextra -pedantic -ggdb
SET SRC=main.c

%CC% %CFLAGS% %SRC% -o swt.exe
