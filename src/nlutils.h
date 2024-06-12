#ifndef CLI_NL_UTILS_H 
#define CLI_NL_UTILS_H

/*
net link utils 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <asm/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/if_addr.h>
#include <linux/if_link.h>

#include <sys/types.h>
#include <sys/ioctl.h> 
#include <sys/socket.h> 

int get_nlsocket();
void netlink_err_msg();

#endif 