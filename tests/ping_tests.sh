#!/bin/sh

ADDR_1=45.33.32.156
ADDR_2=10.1.1.147
ADDR_3=142.250.217.99

make mk-bin-dir ping
./bin/ping -h $ADDR_1 -c 4

sleep 1 

./bin/ping -h $ADDR_2 -s 64

sleep 1 

./bin/ping -h $ADDR_3 -c 8 -s 70 -t 120

sleep 1 

./bin/ping -h $ADDR_3 -t 200



#################################
# tcpdump -vv -t -x -s 1024 icmp -i ens3



: <<'END_COMMENT'
ip hdr 
4500        -- version , ihl  
0054        -- total len  
14b3        -- id 
4000        -- IP flags, (no fragment offset)
40          -- ttl
01          -- protocol (ICMP) 
cca5        -- checksum
0a01 0193   -- src 
2d21 209c   -- dst 

ICMP hdr
0800 -- type (echo) , code (no code) 
a582 -- checksum  
5fbb -- id  
0001 -- sequence 

data 
767c 6866
0000 0000 4d0b 0800 0000 0000 1011 1213
... 
3435 3637

(iphdr)
0x0000:  4500 0054 14b3 4000 4001 cca5 0a01 0193
0x0010:  2d21 209c 

 (icmp)            0800 a582 5fbb 0001 767c 6866
0x0020:  0000 0000 4d0b 0800 0000 0000 1011 1213
0x0030:  1415 1617 1819 1a1b 1c1d 1e1f 2021 2223
0x0040:  2425 2627 2829 2a2b 2c2d 2e2f 3031 3233
0x0050:  3435 3637

END_COMMENT