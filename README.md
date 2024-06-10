# cli-tools 

A collection of command line tools for linux written in C for personal use and to better understand how existing tools, protocols, and concepts work. Some of this software is unfinished, *please* don't have any high expectations. 


## Current tools available 
*(works in progress marked with (\*), may not be fully usable yet)*

* ipinfov1
  * get basic ip info from the kernel using interface addresses
* ipinfov2
  * get more detailed ip info from the kernel using NETLINK sockets
* minserver
  * minimal HTTP 1.1 server for testing static websites 
  * currently supports only GET and POST
  * supports IPv6
* pnginspect
  * inspects the chunks of a png file
* portscan (*)
  * A TCP port scanner that supports the following scan types: 
      * CONNECT
      * SYN
      * NULL 
      * FIN 
      * XMAS
      * ACK 
  * Multi-threading support only available for CONNECT at the moment 
* stegov1
  * hide text inside of a png by appending data after the IEND chunk 

<!-- ## dependencies 
* zlib   -->


## Installation

Running with 
```bash
make 
```
will create all of the tools inside of a directory named bin. 
Usage of all tools can be seen with the flag: -help

