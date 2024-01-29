
SRC_DIR = ./src
BIN_DIR = ./bin

all: port-scanner

port-scanner: 
	gcc -Wall -pthread -o $(BIN_DIR)/portscan $(SRC_DIR)/portscanner.c  

