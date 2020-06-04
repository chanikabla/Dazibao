#ifndef PAIR_H
#define PAIR_H


/** HEADERS **/

#include <time.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "neighbour.h"
#include "util.h"


/** FUNCTION DECLARATIONS **/

void send_neighbour_request(int s, struct sockaddr_in * client, socklen_t client_len);

void send_network_hash(int s, struct sockaddr_in * client, socklen_t client_len);

void decode_msg(int s, char * buff, int rd, struct sockaddr_in * client, socklen_t client_len);

void connection(int argc, char ** argv);

#endif