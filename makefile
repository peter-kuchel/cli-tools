
SRC = src/
BUILD = bin/

CC = gcc
CXX = g++

CFLAGS = -Wall -Wextra -Wcast-align -std=gnu99 -O2
CPPFLAGS = -Wall -Wextra -Wpedantic -std=c++11 -O2

THREADS = -pthread
MATH = -lm

all: 	mk-bin-dir 		\
	 	portscan 		\
	 	ipinfo 			\
	 	ipinfov2 		\
	 	pnginspector 	\
	 	stegoV1 		\
	 	minserver 		\
	 	ping			\
	 	lines			\
	 	grepl			\
	 	obfuscator		\

mk-bin-dir:
	mkdir -p bin 


# CLI TOOLS RULES

PORTSCAN_FILES= $(addprefix $(SRC), inetutils.c common.c portscanner.c)

portscan: $(PORTSCAN_FILES) 
	$(CC) $(CFLAGS) $^ -o $(BUILD)$@ $(THREADS) 


IPINFO1=$(addprefix $(SRC), ipinfoV1.c)

ipinfo: $(IPINFO1)
	$(CC) $(CFLAGS) $< -o $(BUILD)$@


IPINFO2=$(addprefix $(SRC), ipinfoV2.c)

ipinfov2: $(IPINFO2)
	$(CC) $(CFLAGS) $< -o $(BUILD)$@


PNG_INSPECT_FILES= $(addprefix $(SRC), png.c pnginspect.c)

pnginspector: $(PNG_INSPECT_FILES)
	$(CC) $(CFLAGS) $^ -o $(BUILD)$@


STEGOV1=$(addprefix $(SRC), common.c png.c stegoV1.c)

stegoV1: $(STEGOV1)
	$(CC) $(CFLAGS) $^ -o $(BUILD)$@


MINSRV_FILES=$(addprefix $(SRC), inetutils.c common.c logging.c minserver.c)

minserver: $(MINSRV_FILES)
	$(CC) $(CFLAGS) $^ -o $(BUILD)$@ $(MATH) $(THREADS)


PING_FILES=$(addprefix $(SRC), inetutils.c common.c ping.c)

ping: $(PING_FILES)
	$(CC) $(CFLAGS) $^ -o $(BUILD)$@


KCRYPTO_FILES=$(addprefix $(SRC), common.c nlutils.c kcrypto.c)

kcrypto: $(KCRYPTO_FILES)
	$(CC) $(CFLAGS) $^ -o $(BUILD)$@


LINE_FILES=$(addprefix $(SRC), lines.cc)

lines: $(LINE_FILES)
	$(CXX) $(CPPFLAGS) $< -o $(BUILD)$@


GREPL_FILES=$(addprefix $(SRC), grepl.cc)

grepl: $(GREPL_FILES)
	$(CXX) $(CPPFLAGS) $< -o $(BUILD)$@


OBFUS_FILES =$(addprefix $(SRC), obfuscator.cc)

obfuscator: $(OBFUS_FILES)
	$(CXX) $(CPPFLAGS) $< -o $(BUILD)$@