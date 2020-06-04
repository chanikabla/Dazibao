/** HEADERS **/

#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <signal.h>

#include "pair.h"
#include "util.h"
#include "webserver.h"


/** MACROS **/

#define BUFFSIZE 1024
#define PORT 1212
#define TIMER 20
#define MAGIC 0x5F
#define VERSION 1
#define NB_NODE_MAX 16
#define TLV_PAD1 0
#define TLV_PADN 1
#define TLV_NEIGHBOUR_REQUEST 2
#define TLV_NEIGHBOUR 3
#define TLV_NETWORK_HASH 4
#define TLV_NETWORK_STATE_REQUEST 5
#define TLV_NODE_HASH 6
#define TLV_NODE_STATE_REQUEST 7
#define TLV_NODE_STATE 8
#define TLV_WARNING 9
#define NB_NEIGHBOUR_MAX 15


/** VARIABLES **/

extern struct neighbour my_neighbours[NB_NEIGHBOUR_MAX];
extern int nb_neighbours;
extern struct node * node_data;
extern int nb_nodes;
short mySeq = 22236;
#ifdef __APPLE__
unsigned char myId[16]    = "c869cda95b0e0000";
#elif __linux__
unsigned char myId[16]    = "080027d26f9e0000";
#else
unsigned char myId[16]    = "0102030405060708";
#endif


/** FUNCTION DEFINITIONS **/

void fill_header(char * buff) {
    buff[0] = MAGIC;
    buff[1] = VERSION;
    * (short *) &buff[2] = 0;
}

struct sockaddr_in * random_client (socklen_t * client_len) {
    int num_voisin = random() % nb_neighbours; 
    snprintf(strace, 1024, "Client random %d \n", num_voisin);
    trace(strace);

    struct sockaddr_in * client = NULL;
    
    if (my_neighbours[num_voisin].family == AF_INET) {
        client = (struct sockaddr_in*)&my_neighbours[num_voisin].client4;
        *client_len = sizeof(struct sockaddr_in);
    } else if (my_neighbours[num_voisin].family == AF_INET6) {
        client = (struct sockaddr_in*)&my_neighbours[num_voisin].client6;
        *client_len = sizeof(struct sockaddr_in6);
    }

    return client;
}

void send_neighbour_request(int s, struct sockaddr_in * client, socklen_t client_len) {
    trace("send_neighbour_request\n");

    char buff[BUFFSIZE];
    fill_header(buff);

    if (client == 0) 
        client = random_client(&client_len);

    int lg = 6;
    int hdr = 4;
    buff[3] = 2;

    buff[hdr + 0] = TLV_NEIGHBOUR_REQUEST;
    buff[hdr + 1] = 0;

    dump4("neighbour request\n", buff, lg);
    int rc  = sendto(s, buff, lg, 0, (struct sockaddr*) client, client_len);
    snprintf(strace, 1024, "j'ai envoyé %d octets\n", rc);
    trace(strace);

    if (rc == -1) {
        perror("error send dans send_neighbour_request");
    }

    trace("neigbour requet sent !\n");
}

void send_neighbour(int s, struct sockaddr_in * client, socklen_t client_len, int neighbour) {
    trace("send_neighbour\n");

    char buff[BUFFSIZE];
    fill_header(buff);

    int lg = 24;
    int hdr = 4;
    buff[3] = 20;

    buff[hdr + 0] = TLV_NEIGHBOUR;
    buff[hdr + 1] = 18;

    if (my_neighbours[neighbour].family == AF_INET) {
        memcpy(&buff[hdr + 2], (char*) &my_neighbours[neighbour].client4.sin_addr.s_addr, 16);
        memcpy(&buff[hdr + 18], (char*) &my_neighbours[neighbour].client4.sin_port, 2);
    } else if (my_neighbours[neighbour].family == AF_INET6) {
        memcpy(&buff[hdr + 2], (char*) &my_neighbours[neighbour].client6.sin6_addr.s6_addr, 16);
        memcpy(&buff[hdr + 18], (char*) &my_neighbours[neighbour].client6.sin6_port, 2);
    }

    dump4("neighbour\n", buff, lg);
    int rc  = sendto(s, buff, lg, 0, (struct sockaddr*) client, client_len);
    snprintf(strace, 1024, "j'ai envoyé %d octets \n", rc);
    trace(strace);

    if (rc == -1) {
        perror("error send dans send_neighbour");
    }
}

void send_network_hash(int s, struct sockaddr_in * client, socklen_t client_len) {
    trace("send_network_hash\n");

    char buff[BUFFSIZE];
    fill_header(buff);

    int lg = 22;
    int hdr = 4;
    buff[3] = 18;

    buff[hdr + 0] = TLV_NETWORK_HASH;
    buff[hdr + 1] = 16;

    network_hash(nb_nodes, node_data, &buff[hdr + 2]);

    if (client == 0) 
        client = random_client(&client_len);

    dump4("network hash\n", buff, lg);
    int rc  = sendto(s, buff, lg, 0, (struct sockaddr*)client, client_len);
    snprintf(strace, 1024, "j'ai envoyé %d octets \n", rc);
    trace(strace);

    if (rc == -1) {
        perror("error send dans send_network_hash");
    }

    trace("network hash sent !\n");
}

void send_node_hash(int s, struct sockaddr_in * client, socklen_t client_len, int neighbour) {
    trace("send_node_hash\n");

    char buff[BUFFSIZE];
    fill_header(buff);

    int lg = 32;
    int hdr = 4;
    buff[3] = 28;

    buff[hdr + 0] = TLV_NODE_HASH;
    buff[hdr + 1] = 26;

    snprintf(strace, 1024, "node data : %8s ||| %d ||| %s\n", node_data[neighbour].id, node_data[neighbour].seq, node_data[neighbour].data);
    trace(strace);

    memcpy(&buff[hdr + 2], (char*) &node_data[neighbour].id, 8);
    buff[hdr + 11] = node_data[neighbour].seq % 256;
    buff[hdr + 10] = (int) (node_data[neighbour].seq / 256);
    node_hash(node_data[neighbour], &buff[hdr + 12]);
    node_data[neighbour].seq++;

    dump4("node hash\n", buff, lg);
    int rc = sendto(s, buff, lg, 0, (struct sockaddr*) client, client_len);

    if (rc == -1) {
        perror("error send dans send_neighbour");
    }
}

void send_node_state_request(int s, struct sockaddr_in * client, socklen_t client_len, unsigned char * nodeid) {
    trace("send_node_state_request\n");

    unsigned char buff[BUFFSIZE];
    fill_header((char *) buff);

    int lg = 14;
    int hdr = 4;
    buff[3] = 10;

    buff[hdr + 0] = TLV_NODE_STATE_REQUEST;
    buff[hdr + 1] = 8;

    memcpy(&buff[hdr + 2], (char*) nodeid, 8);
    dump4("node state request\n", (char *) buff, lg);
    int rc  = sendto(s, buff, lg, 0, (struct sockaddr*) client, client_len);

    if (rc == -1) {
        perror("error send dans send node state request");
    }
}


void send_node_state(int s, struct sockaddr_in * client, socklen_t client_len, unsigned char * nodeid) {
    trace("send_node_state\n");

    unsigned char buff[BUFFSIZE];
    fill_header((char *) buff);

    int lg = 224;
    int hdr = 4;
    buff[3] = 220;

    buff[hdr + 0] = TLV_NODE_STATE;
    buff[hdr + 1] = 218;

    char hash[16];
    node * znode =  get_node(nodeid);
    if (znode == NULL) {
        snprintf(strace, 1024, "Erreur noeud inconnu < %s >\n", nodeid);
        trace(strace);
        return;
    }

    snprintf(strace, 1024, "node data %s - %d - %s\n", znode->id, znode->seq, znode->data);
    trace(strace);
    memcpy(&buff[hdr +  2], (char*) znode->id, 8);
    buff[hdr + 11] = znode->seq % 256;
    buff[hdr + 10] = (int) (znode->seq / 256);
    node_hash(* znode, hash);
    memcpy(&buff[hdr + 12], (char*) hash, 16);
    memcpy(&buff[hdr + 28], (char*) znode->data, 192);


    dump4("node state\n", (char *) buff, lg);
    int rc  = sendto(s, buff, lg, 0, (struct sockaddr*) client, client_len);

    if (rc == -1) {
        perror("error send dans send note state");
    }
}

void send_network_state_request(int s, struct sockaddr_in * client, socklen_t client_len) {
    trace("send_network_state_request\n");

    char buff[BUFFSIZE];
    fill_header(buff);

    int lg = 6;
    int hdr = 4;
    buff[3] = 2;

    buff[hdr + 0] = TLV_NETWORK_STATE_REQUEST;
    buff[hdr + 1] = 0;

    dump4("network state request\n", buff, lg);
    int rc  = sendto(s, buff, lg, 0, (struct sockaddr*) client, client_len);

    if (rc == -1) {
        perror("error send dans send_network_state_request");
    }
}

void send_network_state(int s, struct sockaddr_in * client, socklen_t client_len) {
    trace("send_network_state\n");

    for (int i=0; i<nb_neighbours; i++) {
        if (my_neighbours[i].etat != -1)
            send_node_hash(s, client, client_len, i);
    }
}

void decode_msg(int s, char * buff, int rd, struct sockaddr_in * client, socklen_t client_len) {
    trace("decode_msg\n");

    if (buff[0] != MAGIC) {
        return;
    }

    if (buff[1] != VERSION) {
        return;
    }

    if (nb_neighbours == NB_NEIGHBOUR_MAX){
        return;
    }

    uint8_t i1 = (uint8_t) buff[2];
    uint8_t i2 = (uint8_t) buff[3];
    short body_length = i2 + 256 * i1;
    snprintf(strace, 1024, "body length %d\n", body_length);
    trace(strace);
    unsigned char * body = (unsigned char *) &buff[4];

    int tlv = 0;

    srandom (time(NULL));
    int neighbourAdded = 0;
    int cpt = 0;

    while (tlv < body_length) {
        uint8_t tlv_lg = 0;
        unsigned char * tlv_ip = NULL;
	    short tlv_port = 0;
        unsigned char * tlv_node_id = NULL;
        short tlv_seqno = 0;
        unsigned char * tlv_node_hash = NULL;
        unsigned char * tlv_network_hash = NULL;
        unsigned char * tlv_msg = NULL;
        unsigned char * tlv_data = NULL;
        char hash[16];
        int neighbour = 0;

        memset (hash, 0, 16);
        snprintf(strace, 1024, "tlv %d body_length %d\n", tlv, body_length);
        trace(strace);
        cpt++;

        if (cpt > 50)
            exit(0);

        if (body[tlv] != TLV_NETWORK_HASH && neighbourAdded == 0) {
            /* Obtain current time. */
            time_t current_time;
            current_time = time(NULL);
            int family = 0;
            if (client_len == sizeof(struct sockaddr_in)) 
                family = AF_INET;
            else if (client_len == sizeof(struct sockaddr_in6)) 
                family = AF_INET6;

            add_neighbour(family, client, 1, current_time);
            neighbourAdded ++;
        }

        switch (body[tlv]) {
            case TLV_PAD1:
                tlv ++;
                break;

            case TLV_PADN:
                tlv_lg = (int) body[tlv+1];
                tlv += tlv_lg + 2;
                break;

            case TLV_NEIGHBOUR_REQUEST:
                trace("recu TLV_NEIGHBOUR_REQUEST\n");
                neighbour = random() % nb_neighbours;
                send_neighbour(s, client, client_len, neighbour);
                tlv += 2;
                break;

            case TLV_NEIGHBOUR:
                trace("recu TLV_NEIGHBOUR\n");
                tlv_ip = (unsigned char *) &buff[tlv+2];
                tlv_port = *(short *)&body[tlv+18];
                tlv += 20;
                send_network_hash(s, client, client_len);
                break;

            case TLV_NETWORK_HASH:
                trace("recu TLV_NETWORK_HASH\n");
                tlv_lg = (int) body[tlv+1];
                tlv_network_hash = &body[tlv+2];
                tlv += 18;
                network_hash(nb_nodes, node_data, hash);
                if (memcmp(hash, tlv_network_hash, 16) != 0) {
                    send_network_state_request(s, client, client_len);
                }
                break;

            case TLV_NETWORK_STATE_REQUEST:
                trace("recu TLV_NETWORK_STATE_REQUEST\n");
                send_network_state(s, client, client_len);
                tlv += 2;
                break;

            case TLV_NODE_HASH:
                trace("recu TLV_NODE_HASH\n");
                tlv_node_id = &body[tlv+2];
                tlv_seqno = body[tlv+10]*256 + body[tlv+11];
                tlv_node_hash = &body[tlv+12];
                node * zn = get_node(tlv_node_id);
                if (zn == NULL) {
                    send_node_state_request(s, client, client_len, tlv_node_id);
                } else {
                    node_hash(*zn, hash);
                    if (memcmp(hash, tlv_node_hash, 8) != 0)
                        send_node_state_request(s, client, client_len, tlv_node_id);
                }
                tlv += 28;
                break;

            case TLV_NODE_STATE_REQUEST:
                trace("recu TLV_NODE_STATE_REQUEST\n");
                tlv_node_id = &body[tlv+2];
                send_node_state(s, client, client_len, tlv_node_id);
                tlv += 10;
                break;

            case TLV_NODE_STATE:
                trace("recu TLV_NODE_STATE\n");
                tlv_lg = (int) body[tlv+1];
                tlv_node_id = &body[tlv+2];
                tlv_seqno = body[tlv+10]*256 + body[tlv+11];
                tlv_node_hash = &body[tlv+12];
                tlv_data = &body[tlv+28];
                zn = get_node(tlv_node_id);
                if (memcmp(tlv_node_id, myId, 8) == 0) {
                    if (compare(zn->seq, tlv_seqno) || zn->seq == tlv_seqno) {
                        zn->seq = (tlv_seqno + 1) % 2^16;
                    }
                } else {
                    if (zn == NULL) {
                        add_node((unsigned char *) tlv_node_id, tlv_seqno, (unsigned char *) tlv_data, 192);
                    } else {
                        node_hash(*zn, hash);
                        if (memcmp(hash, tlv_node_hash, 8) == 0)
                            continue;
                        if (compare (zn->seq, tlv_seqno) == 0) {
                            zn->seq = (tlv_seqno + 1) % 2^16;
                            memcpy(zn->data, tlv_data, 192);
                        }
                    }
                }
                tlv += 2 + tlv_lg;
                dump_nodes();
                break;

            case TLV_WARNING:
                trace("recu TLV_WARNING\n");
                tlv_msg = &body[2];
                tlv_lg = (int) body[tlv+1];
                snprintf(strace, 1024, "Warning  : %s\n", tlv_msg);
                trace(strace);
                tlv += tlv_lg + 2;
                break;

            default:
                break;
        }
    }
}

void connection(int argc, char ** argv) {
    trace("connection\n");
    check_trace();

    write(1, "Quel message voulez vous envoyer ?\n", strlen("Quel message voulez vous envoyer ?\n"));
    unsigned char initMsg[256];
    read(0, initMsg, 256);

    int port = init_neighbour(argc, argv);
    init_node();
    read_myId(myId);
    add_node(myId, mySeq, initMsg, strlen((char *)initMsg));

    int se = socket(AF_INET6, SOCK_DGRAM, 0);
    if (se < 0) {
        perror("erreur socket");
        exit(1);
    }

    int rc = 0;
    struct sockaddr_in6 sin6;

    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = AF_INET6;
    sin6.sin6_port = htons(port);

    int val = 0;
    rc = setsockopt(se, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val));
    snprintf(strace, 1024, "REUSE rc = %d\n", rc);
    trace(strace);

    rc = bind(se, (struct sockaddr*)&sin6, sizeof(sin6));
    if (rc < 0) {
        perror("erreur bind ipv6");
        exit(1);
    }

    //int test = 0;
    int pid1=0; // pid2=0;
    trace("ON ENTRE DANS LE FORK 1\n");
    switch(fork()) {
        case -1:
            perror("error occured while forking 1");
            break;

        case 0:
            // cote client
            pid1=getpid();
            snprintf(strace, 1024, "pid1 %d\n", pid1);
            trace(strace);
            while(1) {
                sleep(TIMER);
                trace("REVEIL\n");
                dump_neighbour();
	
				for (int i=0; i<nb_neighbours; i++) {
				  	if (my_neighbours[i].etat != 1) {
				    	time_t t = time(NULL);
				    	if (my_neighbours[i].date - t > 70) {
				      		if (i==nb_neighbours-1) {
				      			memset(&my_neighbours[i], 0, sizeof(struct neighbour));
				      		} else {				      
								memcpy(&my_neighbours[i], 
										&my_neighbours[i+1], 
										sizeof(struct neighbour) * (nb_neighbours-1 - i));
				      			memset(&my_neighbours[nb_neighbours-1], 0, sizeof(struct neighbour));
				      		}
					      	nb_neighbours--;
				    	}	
				  	}
				}
			    		
				if (nb_neighbours < 5) {
				  	send_neighbour_request(se, 0, 0);
				}

				for (int i=0; i<nb_neighbours; i++) {
				   	send_network_hash(se, 0, 0);
				}
		        break;
			}

        default:
           	break;
    }

    /*
    trace("ON ENTRE DANS LE FORK 2\n");
    switch(fork()) {
        case -1:
            perror("error occured while forking 2");
            break;

        case 0:
            pid2=getpid();
            snprintf(strace, 1024, "pid2 %d\n", pid2);
            trace(strace);
            // connect_tcp();

        default:
            break;
    }
    */

    while(1) {
        dump_neighbour();
        trace("Je vous ecoute !\n");
        unsigned char req[4096];
		struct sockaddr_in6 client;
		socklen_t client_len = sizeof(struct sockaddr_in6);
	    

	    rc = recvfrom(se, req, 4096, 0, (struct sockaddr*)&client, &client_len);
        trace ("je vous decode!\n");
        if(rc<0) {
        	perror("erreur recvfrom");
        }
        
        snprintf(strace, 1024, "j'ai recu rc = %d\n", rc);
        trace(strace);

        rc = recvfrom(se, req, 4096, 0, (struct sockaddr*)&client, &client_len);
        trace ("je vous decode!\n");
        if(rc<0) {
        	perror("erreur sendto");
        }

        snprintf(strace, 1024, "j'ai recu rc = %d\n", rc);
        trace(strace);

        dump4("message recu\n", (char *) req, rc);
        decode_msg(se, (char *) req, rc, (struct sockaddr_in*) &client, client_len);
    }
    
}
