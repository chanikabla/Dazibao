/** HEADERS **/

#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#include "neighbour.h"
#include "util.h"


/** MACROS **/

#define NB_NEIGHBOUR_MAX 15


/** VARIABLES **/
struct neighbour my_neighbours[NB_NEIGHBOUR_MAX];
int nb_neighbours;
static char * dazibaoName = "jch.irif.fr";

/** FUNCTION DEFINITIONS **/

void dump_neighbour() {
    trace("dump_neighbour\n");

    if (nb_neighbours == 0) {
        trace("Il n'y a aucun voisin !\n");
    }

    for (int i=0; i<nb_neighbours; i++) {
        char timeString[8];
        struct tm * time_info = localtime(&my_neighbours[i].date);

        strftime(timeString, 8, "%H:%M:%S", time_info);
        sprintf (strace, "%d : ", i);
        trace(strace);

        if (my_neighbours[i].family == AF_INET) {
            snprintf(strace, 1024, " : AF_INET  : ");
            unsigned char ip[4];
            memcpy(&ip, &my_neighbours[i].client4.sin_addr.s_addr, 4);
            for (int j= 0; j<4; j++) {
                sprintf (strace, "%02d ", ip[j]);
                trace(strace);
            }

            sprintf(strace, " %d %d %8s\n",
                    my_neighbours[i].client4.sin_port,
                    my_neighbours[i].etat,
                    timeString);
            trace(strace);

        } else {

            trace(" : AF_INET6 : ");
            for (int j= 0; j<16; j++) {
                sprintf (strace, "%02X ", my_neighbours[i].client6.sin6_addr.s6_addr[j]);
                trace(strace);
            }

            sprintf(strace, " %d %d %8s\n",
                    my_neighbours[i].client6.sin6_port,
                    my_neighbours[i].etat,
                    timeString);
            trace(strace);

        }
    }
}

int init_neighbour(int argc, char ** argv) {
    trace("init_neighbour\n");

    nb_neighbours = 0;

    for (int i=0; i<NB_NEIGHBOUR_MAX; i++) {
        my_neighbours[i].etat = -1;
        my_neighbours[i].date = time(NULL);
    }

    char * servername = NULL;
    char * port = NULL;

    if (argc == 1) { 
		servername = dazibaoName; 
		port = "1212";
    }

    if (argc == 2) {
    	servername = argv[1];
    	port = "1212";
    }

    if (argc == 3) {
    	servername = argv[1];
    	port = argv[2];
    }

    if (argc > 3) {
    	snprintf(strace, 1024, "Usage : main | main + servername | main + servername + port");
    	trace(strace);
    }

	snprintf(strace, 1024, "On essaie avec le serveur %s, au port %s...\n", servername, port);
	trace(strace);

	struct addrinfo hints, *res0, *res;
	int status = 0;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6; 
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_V4MAPPED | AI_ALL;

    if ((status = getaddrinfo(servername, port, &hints, &res0)) != 0) {
        perror("Erreur getaddrinfo");
    }

    for (res = res0; res != NULL; res = res->ai_next) {
        char ipstr[INET6_ADDRSTRLEN], ipver = 0;
        struct sockaddr *addr = NULL;

        if (res->ai_family == AF_INET) { // IPv4
            // struct sockaddr_in *ipv4 = (struct sockaddr_in *)res->ai_addr;
            // addr = &(ipv4->sin_addr);
            addr = (struct sockaddr *) res->ai_addr;
            ipver = '4';
            add_neighbour(res->ai_family, addr, 0, time(NULL));

        } else if (res->ai_family == AF_INET6) { // IPv6
            // struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)res->ai_addr;
            // addr = &(ipv6->sin6_addr);
            addr = (struct sockaddr *) res->ai_addr;
            ipver = '6';
            add_neighbour(res->ai_family, addr, 0, time(NULL));
        }

        inet_ntop(res->ai_family, addr, ipstr, sizeof ipstr);
        snprintf(strace, 1024, "IPv%c: %s\n", ipver, ipstr);
        trace(strace);

    }
	
	return atoi(port);
}

void add_neighbour(uint8_t family, void * addr, short etat, time_t date) {
    trace("add_neighbour\n");

    if (nb_neighbours == NB_NEIGHBOUR_MAX) {
        return;
    }

    for (int i=0; i<nb_neighbours; i++)  {
        if (family == AF_INET) {
            struct sockaddr_in * addr4 = (struct sockaddr_in *) addr;
            if (memcmp(&my_neighbours[i].client4.sin_addr, &addr4->sin_addr, sizeof(struct in_addr)) == 0) {
                my_neighbours[i].date = date;
                return;
            }
        } else if (family == AF_INET6) {
            struct sockaddr_in6 * addr6 = (struct sockaddr_in6 *) addr;
            if (memcmp(&my_neighbours[i].client6.sin6_addr, &addr6->sin6_addr, sizeof(struct in6_addr)) == 0) {
                my_neighbours[i].date = date;
                return;
            }
        }
    }

    my_neighbours[nb_neighbours].date = date;
    my_neighbours[nb_neighbours].etat = etat;
    my_neighbours[nb_neighbours].family = family;
    
    if (family == AF_INET) {
         memcpy(&my_neighbours[nb_neighbours].client4, addr, sizeof(struct sockaddr_in));
        nb_neighbours++;
    } else if (family == AF_INET6) {
         memcpy(&my_neighbours[nb_neighbours].client6, addr, sizeof(struct sockaddr_in6));
        nb_neighbours++;
    } else {
        printf("Bizarre AF_INET inconnu\n");
    }

    dump_neighbour();
}

void clean_neighbour() {
    trace("clean_neighbour\n");

    time_t current_time;
    current_time = time(NULL);
    
    for (int i=0; i<nb_neighbours; i++) {
        if (my_neighbours[i].etat == 0)
            continue;
        unsigned long seconds = difftime (current_time, my_neighbours[i].date);
        if (seconds < 70)
            continue;
        if (i < nb_neighbours -1)
            memcpy(&my_neighbours[i], &my_neighbours[i+1], (nb_neighbours-i-1*sizeof(struct neighbour)));
        else
            memset(&my_neighbours[i], 0, sizeof(struct neighbour));
    }

}
