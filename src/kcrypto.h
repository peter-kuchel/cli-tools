#ifndef CLI_KCRYPTO_H 
#define CLI_KCRYPTO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include <sys/types.h>
#include <sys/ioctl.h> 
#include <sys/socket.h> 

#include <linux/if_alg.h>
#include <linux/netlink.h>
#include <asm/types.h>

#include "nlutils.h"


#define SHA_256_SIZE        32


typedef struct {
    uint8_t include_opts;
    int type
} kcryptopts;


#endif