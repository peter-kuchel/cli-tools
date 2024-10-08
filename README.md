# cli-tools 

A collection of command line tools written in C/C++ for personal use and to better understand how existing tools, protocols, and concepts work. Some of this software is unfinished, *please* don't have any high expectations. 


## Current tools available 
*(major works in progress marked with (\*), though some tools do not offer full functionality at the moment)*

* grepl 
  * a simple regex tool which supports the following patterns:
      * c  - match any literal character c
      * .  - wildcard, match any character
      * [] and ^[] - match positive and negative character groups
      * \+  - match one or more occurances of the previous character
      * ?  - match zero or one occurances of the previous character
      * \w - match any alphanumeric character 
      * \d - match any digit   
      * ^  - match start of line
      * $  - match end of line 
      * |  - match alternate pattern such as (x|y)
      * (...) - matching patterns in groups
        * supports single and multiple backreferences
  * supports combining multiple patterns together
   
* ipinfov1
  * get basic ip info from the kernel using interface addresses

* ipinfov2
  * get more detailed ip info from the kernel using NETLINK sockets

* kcrypto (\*)
  * use the linux kernel crypto functionality via netlink sockets

* lines 
  * simple tool to get line and size numbers for a project 

* minserver
  * minimal HTTP 1.1 server for testing static websites 
  * currently supports only GET and POST
  * supports IPv6

* obfuscator (\*)
  * remove whitespace and rename variables of code files 
  * supports obfuscating the following languages:  
    * JavaScript

* ping 
  * simple ping tool that uses ICMP to check if a host is online (only ipv4 at the moment)
  * currently doesn't support DNS resolution of site names (needs ipv4 address as input)

* pnginspect
  * inspects the chunks of a png file

* portscan
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

Running  
```bash
make 
```
will create all of the tools inside of a directory named bin. 
Tools can be build individually by inspecting the makefile. 

Usage of all tools should be seen with the flag: -help or -h , or by running the binary with no input. 

NOTE: currently all of the tool have been developed and tested on Linux, there are no guarantees they will work elsewhere


