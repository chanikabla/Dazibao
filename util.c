/** HEADERS **/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

#include "util.h"
#include "sha256.h"

int mode_trace = 0;
int ftrace = 0;
char ftracename[256];

/** FUNCTION DEFINITIONS **/

void check_trace() {
    if (open ("./trace", O_RDONLY) > 0) 
    mode_trace = 1;

    if (open ("./trace2", O_RDONLY) > 0) 
        mode_trace = 2;

    time_t date = time(NULL);
    struct tm  *tm = localtime(&date);

    snprintf(ftracename, 256, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
    strncat(ftracename, "_trace.txt", 11);
    ftrace = open (ftracename, O_CREAT|O_WRONLY, 0777);
    if (ftrace <= 0) {
        perror("Erreur ouverture fichier trace");
        printf("%s\n", ftracename);
    }
}

void trace(char * s) {
    if (mode_trace == 0)
        return;
    write(ftrace, s, strlen(s));
    if (mode_trace == 2)
        write(1, s, strlen(s));
}

void dump16(char * header, unsigned char s[16]) {
    trace(header);

    for (int j= 0; j<16; j++) {
        sprintf (strace, "%02X ", s[j]);
        trace (strace);
    }

    trace("\n");
}

void dump4(char * header, char * s, int l) {
    trace (header);

    unsigned char * ss = (unsigned char *)s;

    for (int j= 0; j<l; j++) {
        sprintf (strace, "%02X ", ss[j]);
        trace (strace);
        if (j % 4 == 3)
            trace("\n");
    }

    trace("\n");
}

int sum(int s, int n) {
    return ((s+n)%(2^16));
}

int compare(int s1, int s2) { // s1 <= s2 
    if (((s2-s1)%65536)<32768)
        return 1;
    return 0;
}

void h(char *c, size_t len, char output[16]) {
    unsigned char h[256];
    sha256_init(&ctx);
    sha256_update(&ctx, (const BYTE *)c, len);
    sha256_final(&ctx, h);
    memcpy(output, h, 16);
}

void node_hash(node n, char output[16]) {
    char data[202];
    memcpy(data, (char *)&n, 202);
    // inversion des deux octets pour format gros bout
    char a = data[8];
    data[8] = data[9];
    data[9] = a;
    // hash
    h(data, sizeof(n), output);
}

void network_hash(int nb_node, node *l_node, char output[16]) {
    char * buff = (char *) malloc(16 * nb_node);
    if (nb_node > 1)
        qsort (l_node, nb_node, sizeof(node), sort_node);
    for (int i=0; i<nb_node; i++) {
        h((char *) &l_node[i], sizeof(node), &buff[16*i]);
    }
    h(buff, 16 * nb_node, output);
    free (buff);
}
