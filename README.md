# cli-tools 

A collection of command line tools written in C for personal use and to better understand how existing tools, protocols, and concepts work. 


## current tools available 

* List ipinfov1
  * List get basic ip info from the kernel  
* List ipinfov2
  * List get more detailed ip info from the kernel
* List pnginspect
  * List inspects the chunks of a png file
* List portscan 
  * List port scanner that supports vanilla and SYN scans 
* List stegov1
  * List hide text inside of a png by appending data after the IEND chunk 

## installation

Running with 
```bash
make 
```
will create all of the tools inside of a directory named bin