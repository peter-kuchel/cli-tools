
SRC_DIR = src
BIN_DIR = bin

CC = gcc
CXX = g++

CFLAGS = -Wall -Wextra -Wcast-align -std=gnu99 -O2
CPPFLAGS = -Wall -Wextra -Wpedantic -std=c++11 -O2

THREADS = -pthread
MATH = -lm

COMMONS = 	$(SRC_DIR)/common.c
IMGS	= 	$(SRC_DIR)/png.c
CRYPTO 	= 	$(SRC_DIR)/kcrypto.clnet
INET =  	$(SRC_DIR)/inetutils.c
LOG  =  	$(SRC_DIR)/logging.c
NTL  =      $(SRC_DIR)/nlutils.c

all: 	mk-bin-dir 		\
	 	port-scanner 	\
	 	ip-info 		\
	 	ip-info-v2 		\
	 	png-inspector 	\
	 	stego-v1 		\
	 	minserver 		\
	 	ping			\
	 	lines			\

mk-bin-dir:
	mkdir -p bin 

# HEADERS AND FUNCTIONS ?

# CLI TOOLS 
port-scanner: 
	$(CC) $(CFLAGS) $(COMMONS) $(THREADS) $(INET) -o $(BIN_DIR)/portscan $(SRC_DIR)/portscanner.c  

ip-info: 
	$(CC) $(CFLAGS) -o $(BIN_DIR)/ipinfov1 $(SRC_DIR)/ipinfoV1.c

ip-info-v2:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/ipinfov2 $(SRC_DIR)/ipinfoV2.c

png-inspector:
	$(CC) $(CFLAGS) $(IMGS) -o $(BIN_DIR)/pnginspect $(SRC_DIR)/pnginspect.c

stego-v1: 
	$(CC) $(CFLAGS) $(COMMONS) $(IMGS) $(SRC_DIR)/stegoV1.c -o $(BIN_DIR)/stegov1

minserver:
	$(CC) $(CFLAGS) $(MATH) $(THREADS) $(COMMONS) $(LOG) $(INET) $(SRC_DIR)/minserver.c -o $(BIN_DIR)/minserver

ping:
	$(CC) $(CFLAGS) $(INET) $(COMMONS) $(SRC_DIR)/ping.c -o $(BIN_DIR)/ping

kcrypto: 
	$(CC) $(CFLAGS) $(COMMONS) $(NTL) $(SRC_DIR)/kcrypto.c -o $(BIN_DIR)/kcrypto

lines:
	$(CXX) $(CPPFLAGS) $(SRC_DIR)/lines.cpp -o $(BIN_DIR)/lines 


