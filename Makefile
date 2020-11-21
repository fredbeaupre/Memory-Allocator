# .DEFAULT_GOAL=all
CC=gcc
DEPS=sma.h

sma: a3_test.c sma.c
	$(CC) -o sma a3_test.c sma.c


