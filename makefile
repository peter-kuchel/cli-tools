
SRC_DIR = ./src
BIN_DIR = ./bin

all: port-scanner test-ipv6 stego-pic 

port-scanner: 
	gcc -Wall -pthread -o $(BIN_DIR)/portscan $(SRC_DIR)/portscanner.c  

test-ipv6: 
	gcc -Wall -o $(BIN_DIR)/testipv6 $(SRC_DIR)/testipv6.c 

stego-pic:
	gcc -Wall -o $(BIN_DIR)/stego $(SRC_DIR)/stego.c

