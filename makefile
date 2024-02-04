
SRC_DIR = src
BIN_DIR = bin

CC = gcc
CFLAGS = -Wall -Wextra -std=gnu99

COMMONS = $(SRC_DIR)/common.c
IMGS	= $(SRC_DIR)/png.c 

all: desc port-scanner ip-info png-inspector stego-pic 

desc: 
	$(CC) -o tooldesc $(SRC_DIR)/tooldesc.c  
	
port-scanner: 
	$(CC) $(CFLAGS) -pthread -o $(BIN_DIR)/portscan $(SRC_DIR)/portscanner.c  

ip-info: 
	$(CC) $(CFLAGS) -o $(BIN_DIR)/ipinfo $(SRC_DIR)/ipinfo.c

png-inspector:
	$(CC) $(CFLAGS) $(IMGS) -o $(BIN_DIR)/pnginspect $(SRC_DIR)/pnginspect.c

stego-pic: 
	$(CC) $(CFLAGS) $(COMMONS) $(SRC_DIR)/stegoV1.c -o $(BIN_DIR)/stego

