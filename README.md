# cli-tools 

A collection of command line tools written in C for personal use and to better understand how existing tools, protocols, and concepts work. 


## current tools available 

* ipinfov1
  * get basic ip info from the kernel using interface addresses
* ipinfov2
  * get more detailed ip info from the kernel using NETLINK sockets
* pnginspect
  * inspects the chunks of a png file
* portscan 
  * port scanner that supports vanilla and SYN scans 
* stegov1
  * hide text inside of a png by appending data after the IEND chunk 

## installation

Running with 
```bash
make 
```
will create all of the tools inside of a directory named bin