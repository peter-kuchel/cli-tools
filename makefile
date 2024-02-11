
SRC_DIR = src
BIN_DIR = bin

CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99 -g

COMMONS = $(SRC_DIR)/common.c
IMGS	= $(SRC_DIR)/png.c
CRYPTO 	= $(SRC_DIR)/kcrypto.c 
NET		= $(SRC_DIR)/netutils.c 

all: desc port-scanner ip-info ip-info-v2 png-inspector stego-v1 

desc: 
	$(CC) -o tooldesc $(SRC_DIR)/tooldesc.c  
	
port-scanner: 
	$(CC) $(CFLAGS) $(COMMONS) $(NET) -pthread -o $(BIN_DIR)/portscan $(SRC_DIR)/portscanner.c  

ip-info: 
	$(CC) $(CFLAGS) -o $(BIN_DIR)/ipinfov1 $(SRC_DIR)/ipinfoV1.c

ip-info-v2:
	$(CC) $(CFLAGS) -o $(BIN_DIR)/ipinfov2 $(SRC_DIR)/ipinfoV2.c

png-inspector:
	$(CC) $(CFLAGS) $(IMGS) -o $(BIN_DIR)/pnginspect $(SRC_DIR)/pnginspect.c

stego-v1: 
	$(CC) $(CFLAGS) $(COMMONS) $(IMGS) $(SRC_DIR)/stegoV1.c -o $(BIN_DIR)/stegov1

