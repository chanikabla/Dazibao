/** HEADERS **/

#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/sysctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "util.h"
#include "node.h"

#ifdef __APPLE__
#include <net/if_types.h>
#include <net/if_dl.h>
#endif


/** MACROS **/

#define NB_NODE_MAX 256
#define AF_LINK 18
#define NET_RT_IFLIST 3


/** VARIABLES **/

struct node * node_data;
int nb_nodes;


/** FUNCTION DEFINITIONS **/

void dump_nodes() {
    trace("== NODES ==\n");
    for (int i=0; i<nb_nodes; i++) {
        snprintf(strace, 1024, "%03d || ", i);
        trace(strace);
        for (int j=0; j<8; j++) {
            snprintf(strace, 1024, "%02X", (unsigned char) node_data[i].id[j]);
            trace(strace);
        }
        snprintf(strace, 1024, " | %5d | %40s\n", node_data[i].seq, node_data[i].data);
        trace(strace);
    }
    trace("== END NODES ==\n");
}

void init_node() {
    nb_nodes = 0;
    node_data = (struct node *)malloc(NB_NODE_MAX * sizeof(struct node));
    for (int i=0; i<NB_NODE_MAX; i++) {
        memset(&node_data[i].id, 0, 8) ;
        memset(&node_data[i].data, 0, 192);
        node_data[i].seq = 0;
    }
}

node * get_node(unsigned char * nodeid) {
    for (int i=0; i<nb_nodes; i++) {
        if (memcmp(node_data[i].id, nodeid, 8) == 0) {
            return & node_data[i];
        }
    }
    return NULL;
}

void update_node(unsigned char * nodeid, short seq, unsigned char * data) {
    node * n = get_node(nodeid);
    if (n == NULL) {
        printf("Erreur update node id %s\n", nodeid);
        return;
    }
    n->seq = seq;
    memcpy(n->data, data, 192);
}

void add_node(unsigned char * nodeId, short seq, unsigned char * data, int lgdata) {
    for (int i=0; i<nb_nodes; i++) {
        if (memcmp(node_data[i].id, nodeId, 8) == 0) {
	    memcpy(&node_data[i], data, lgdata);
            node_data[i].seq =  seq;
            return;
        }
    }
    
    if (nb_nodes == NB_NODE_MAX)
        return;

    memcpy(node_data[nb_nodes].id, nodeId, 8);
    memcpy(node_data[nb_nodes].data, data, lgdata);
    node_data[nb_nodes].seq = seq;
    nb_nodes++;
    dump_nodes();
}

int sort_node (const void * n1, const void * n2) {
    node * nn1 = (node *) n1;
    node * nn2 = (node *) n2;
    return strncmp(nn1->id, nn2->id, 8);
}

#ifdef __APPLE__
void read_myId(unsigned char id[16]) {
    int                 mib[6];
    size_t              len;
    char                *buf;
    unsigned char       *ptr;
    struct if_msghdr    *ifm;
    struct sockaddr_dl  *sdl;

    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;

    char * interface = "en0";
    
    if ((mib[5] = if_nametoindex(interface)) == 0) {
        return;
    }

    if (sysctl(mib, 6, NULL, &len, NULL, 0) < 0) {
        perror("sysctl 1 error");
        exit(3);
    }

    if ((buf = malloc(len)) == NULL) {
        perror("malloc error");
        exit(4);
    }

    if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
        perror("sysctl 2 error");
        exit(5);
    }

    ifm = (struct if_msghdr *)buf;
    sdl = (struct sockaddr_dl *)(ifm + 1);
    ptr = (unsigned char *)LLADDR(sdl);
    snprintf((char *) id, 16, "%02x%02x%02x%02x%02x%02x0000", *ptr, *(ptr+1), *(ptr+2),
            *(ptr+3), *(ptr+4), *(ptr+5));
}
#endif

#ifdef __linux__
#define HWADDR_len 6
void read_myId(unsigned char id[16]) {
	int s,i;
    struct ifreq ifr;
    s = socket(AF_INET, SOCK_DGRAM, 0);
    ioctl(s, SIOCGIFHWADDR, &ifr);

    for (i=0; i<HWADDR_len; i++)
        sprintf((char *)id,"%02x%02x%02x%02x%02x%02x000",((unsigned char*)ifr.ifr_hwaddr.sa_data)[0], ((unsigned char*)ifr.ifr_hwaddr.sa_data)[1], ((unsigned char*)ifr.ifr_hwaddr.sa_data)[2], ((unsigned char*)ifr.ifr_hwaddr.sa_data)[3], ((unsigned char*)ifr.ifr_hwaddr.sa_data)[4], ((unsigned char*)ifr.ifr_hwaddr.sa_data)[5]);
}
#endif
