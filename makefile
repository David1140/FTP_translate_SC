#FLAG=-Wall -Werror
#STD=-std=gnu99
SHELL := /bin/bash
CC := gcc
GDB := -g

all:
	${CC} ${GDB} FTP_server.c agreement.c pthread.c -o FTP_server -lpthread
	${CC} ${GDB} FTP_client.c agreement.c pthread.c -o FTP_client -lpthread
	${CC} ${GDB} fortest_client.c -o fortest_client

clean:
	rm FTP_server FTP_client
	bash clean.sh