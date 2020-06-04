#ifndef NODE_H
#define NODE_H

/** DATA TYPES **/

typedef struct node {
    char id[8]; // id du noeud
    short seq; // numéro de séquence du noeud
    char data[192]; // data du noeud
} node;


/** FUNCTION DECLARATIONS **/

void dump_nodes();

void init_node();

void read_myId(unsigned char id[16]);

void add_node(unsigned char * nodeId, short seq, unsigned char * data, int lgdata);

int sort_node (const void * n1, const void * n2);

node * get_node(unsigned char * nodeid);

void update_node(unsigned char * nodeid, short seq, unsigned char * data);

#endif
