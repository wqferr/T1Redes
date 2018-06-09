#include "net/address.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

union intbytes {
    uint8_t bytes[4];
    uint32_t value;
};

int lit_end_host(void);
void writebytes(const char *ipstr, union intbytes *out);


uint32_t iptoint(const char *ipstr) {
    union intbytes ip;
    writebytes(ipstr, &ip);
    return ip.value;
}



void writebytes(const char *ipstr, union intbytes *out) {
    if (lit_end_host()) {
        sscanf(ipstr, "%hhu.%hhu.%hhu.%hhu",
            &out->bytes[3],
            &out->bytes[2],
            &out->bytes[1],
            &out->bytes[0]);
    } else {
        sscanf(ipstr, "%hhu.%hhu.%hhu.%hhu",
            &out->bytes[0],
            &out->bytes[1],
            &out->bytes[2],
            &out->bytes[3]);
    }
}

// Checks if host is little endian
int lit_end_host(void) {
    int one = 1;
    char *test = (char *) &one;
    return (*test) > 0;
}