#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <sys/types.h> 
#include <sys/socket.h> 

#include "inetutils.h"
/* 
    checksum taken from: 
    https://locklessinc.com/articles/tcp_checksum/ 
*/
unsigned short checksum(const char *buf, unsigned size)
{
	unsigned sum = 0;
	unsigned i;

	/* Accumulate checksum */
	for (i = 0; i < size - 1; i += 2)
	{
		unsigned short word16 = *(unsigned short *) &buf[i];
		sum += word16;
	}

	/* Handle odd-sized case */
	if (size & 1)
	{
		unsigned short word16 = (unsigned char) buf[i];
		sum += word16;
	}

	/* Fold to get the ones-complement result */
	while (sum >> 16) sum = (sum & 0xFFFF)+(sum >> 16);

	/* Invert to get the negative in ones-complement arithmetic */
	return ~sum;
}


in_addr_t find_src_inet_addr(){
	in_addr_t addr = 0; 

	struct ifaddrs *curr_ifa, *init_ifa;
	struct sockaddr* ifa_saddr;

	if (getifaddrs(&init_ifa) == -1){
        perror("getifaddrs: could not init interface address struct\n");
		return addr; 
    }      

	curr_ifa = init_ifa; 
	while (curr_ifa != NULL){

		ifa_saddr = curr_ifa->ifa_addr;
		if (
			ifa_saddr != NULL 						&& 		// check that interface addr is not null
			strcmp(curr_ifa->ifa_name, "lo") != 0 	&& 		// check that interface is not local 
			ifa_saddr->sa_family == AF_INET 				// check that address is ipv4
		){					
			addr =  ((struct sockaddr_in*)ifa_saddr)->sin_addr.s_addr;	// found inet addr, assume is the one we want
			break; 
		}

		curr_ifa = curr_ifa->ifa_next;
	}

	return addr; 

}