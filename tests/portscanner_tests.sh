#!/bin/sh

# for testing for real (scanme.nmap.org): 45.33.32.156 - allowed to scan the site 
REAL_HOST=45.33.32.156

LOCAL_HOST=127.0.0.1

# compile port scanner 
make mk-bin-dir port-scanner


printf "[beginning type-tests on $LOCAL_HOST]"
sleep 1
./bin/portscan -host=$LOCAL_HOST -port=5432 
sleep 1
./bin/portscan -host=$LOCAL_HOST -port=5432 -type=SYN
sleep 1 
./bin/portscan -host=$LOCAL_HOST -port=5432 -type=ACK
sleep 1
./bin/portscan -host=$LOCAL_HOST -port=5432 -type=FIN
sleep 1 
./bin/portscan -host=$LOCAL_HOST -port=5432 -type=NULL 
sleep 1 
./bin/portscan -host=$LOCAL_HOST -port=5432 -type=XMAS

# printf "beginning tests on scanme.nmap.org" 
# ./bin/portscan -host=$REAL_HOST -port=22 -type=SYN

# ##########################################################################################
# nmap -sS 127.0.0.1 -Pn -p 11309
# tcpdump -vv -t -x -s 4096 -i lo -Qinout 'port 5432'

# todo 
# get the interface and address of the machine 
# need to check that the resp is from the correct src after the probe
# fix the issues with sending outside of localhost -- should be fixed with getting the correct interface address 


# example packet breakdown from tcpdump
# 45 00 : ihl & version , tos   
# 002c  : total len
# bfb3  : id 
# 0000  : fragment offset 
# 28 06 : ttl, protocol 
# d516  : ip checksum  
# 7f00 0001 : src addr 
# 7f00 0001 : dst addr
# eb67 : src port 
# 2c2d : dst port 
# 22b9 92b5 : sequence num
# 0000 0000 : acknowledgement num
# 6 0 02     : word offset, reserved, TCP flags     
# 0400 : window size  
# c920 : checksum  
# 0000 : urgent pointer 
# 0204 05b4 : mss option, size 