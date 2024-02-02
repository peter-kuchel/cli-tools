
SRC_DIR = src
BIN_DIR = bin

CFLAGS = -Wall -Wextra -std=gnu99

all: desc port-scanner ip-info stego-pic 

desc: 
	gcc -o tooldesc $(SRC_DIR)/tooldesc.c 
	
port-scanner: 
	gcc $(CFLAGS) -pthread -o $(BIN_DIR)/portscan $(SRC_DIR)/portscanner.c  

ip-info: 
	gcc $(CFLAGS) -o $(BIN_DIR)/ipinfo $(SRC_DIR)/ipinfo.c

stego-pic:
	gcc $(CFLAGS) -o $(BIN_DIR)/stego $(SRC_DIR)/stego.c

