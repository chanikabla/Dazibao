#ifndef UTIL_H
#define UTIL_H

/** HEADERS **/

#include <stddef.h>
#include <stdio.h>

#include "node.h"
#include "sha256.h"


/** VARIABLES **/


char strace[1024];
SHA256_CTX ctx;


/** FUNCTION DECLARATIONS **/

void check_trace();

void trace(char * s);

void dump16(char * heaer, unsigned char s[16]);

void dump4(char * header, char * ss, int l);

int sum(int s, int n);

int compare(int s1, int s2);

void h(char *c, size_t len, char output[16]);

void node_hash(node n, char output[16]);

void network_hash(int nb_node, node *l_node, char output[16]);
#endif
