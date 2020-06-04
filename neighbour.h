#ifndef NEIGBOUR_H
#define NEIGBOUR_H
/** DATA TYPES **/

typedef struct neighbour {
    short etat; // 0 pair permanent | 1 transitoire | -1 etat inconnu
    time_t date;
    uint8_t family;
    struct sockaddr_in client4;
    struct sockaddr_in6 client6;
} neighbour;


/** FUNCTION DECLARATIONS **/

void dump_neighbour();

void add_neighbour(uint8_t family, void * addr, short etat, time_t date);

int init_neighbour(int argc, char ** argv);

void clean_neighbour();

#endif
