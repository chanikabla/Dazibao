/** HEADERS **/
#include <assert.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "node.h"
#include "webserver.h"

#define BUFFSIZE 1024


/** VARIABLES **/

extern struct node * node_data;
extern int nb_nodes;


/** FUNCTION DEFINITIONS **/

void success_200(int connection) {
    char * answer = "HTTP/1.0 200 OK\n";
    write(connection, answer, strlen(answer));
}

void error_400(int connection) {
    char * answer = "HTTP/1.0 400 Bad Request\n";
    write(connection, answer, strlen(answer));
}

void error_404(int connection) {
    char * answer = "HTTP/1.0 404 Not Found\n";
    write(connection, answer, strlen(answer));
}

void node_html(char *s, int lmax) {
    char str[256];
    memset(str, 0, 256);
    s[0] = 0;

    strncat(s, "<html>", strlen("<html>"));
    strncat(s, "<body>", strlen("<body>"));
    strncat(s, "<table>\n", strlen("<table>\n"));
    strncat(s, "<tr>", strlen("<tr>"));
    strncat(s, "<td><b>Id</b></td>", strlen("<td><b>Id</b></td>"));
    strncat(s, "<td><b>Seq</b></td>", strlen("<td><b>Seq</b></td>"));
    strncat(s, "<td><b>Data</b></td>", strlen("<td><b>Data</b></td>"));
    strncat(s, "</tr>\n", strlen("</tr>\n"));

    for (int i=0; i<nb_nodes; i++) {
        strncat(s, "<tr>", strlen("<tr>"));
        strncat(s, "<td>", strlen("<td>"));
        for (int j=0; j<8; j++) {
            sprintf(str, "%02X", (unsigned char) node_data[i].id[j]);
            strncat(s, str, strlen(str));
        }
        strncat(s, "</td>", strlen("</td>"));
        strncat(s, "<td>", strlen("<td>"));
        sprintf(str, "%d", node_data[i].seq);
        strncat(s, str, strlen(str));
        strncat(s, "</td>", strlen("</td>"));
        strncat(s, "<td>", strlen("<td>"));
        strncat(s, node_data[i].data, strlen(node_data[i].data));
        strncat(s, "</td>", strlen("</td>"));
        strncat(s, "</tr>\n", strlen("</tr>\n"));
    }

    strncat(s, "</table>", strlen("</table>"));
    strncat(s, "</body>\n", strlen("</body>\n"));
    strncat(s, "</html>\n", strlen("</html>\n"));
}

void run(int s_client) {
    char buff[BUFFSIZE];
    memset(buff, 0, BUFFSIZE);

    // char header[] = "GET";
    char * arg = NULL;

    while (1) {
        int rd = read(s_client, buff, BUFFSIZE);
        if (rd < 0) {
            perror ("error reading s_client");
            close(s_client);
            break;
        }

        printf("IL ETAIT UNE FOIS CE BUFFER: %s\n", buff);
        arg = strtok(buff, " ");
        // char * p = NULL;

        char  * str1 = "HTTP/1.1 200 OK \n\
        Content-Type: text/html\n";

        char * str2 = "<html>\n\
        <body>\n\
        <h1>Hello, World!</h1>\n\
        </body>\n\
        </html>\n";

        char str3[10000];
        memset(str3, 0, 10000);

        node_html(str3, 10000);
        printf("%s\n", str3);

        int rc = 0;
        rc = write (s_client, str1, strlen(str1));
        assert(rc>0);
        printf("rc = %d\n", rc);
        rc = write (s_client, str2, strlen(str2));
        assert(rc>0);
        printf("rc = %d\n", rc);
        rc = write (s_client, str3, strlen(str3));
        assert(rc>0);
        printf("rc = %d\n", rc);
        fsync(s_client);
    }
}

void connect_tcp() {
    int s_server = socket(PF_INET6, SOCK_STREAM, 0);
    int s_client = 0;

    if (s_server < 0) {
        perror("error creating socket");
        exit(1);
    }

    struct sockaddr_in6 sin6;
    memset(&sin6, 0, sizeof(sin6));
    sin6.sin6_family = PF_INET6;
    sin6.sin6_port = htons(4242);

    int val = 0;
    int rc = setsockopt(s_server, IPPROTO_IPV6, IPV6_V6ONLY, &val, sizeof(val));
    printf("IPV6 rc = %d\n", rc);
    val = 1;
    rc = setsockopt(s_server, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
    printf("REUSE rc = %d\n", rc);

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    rc = setsockopt(s_server, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
    if (rc < 0)
        perror("setsockopt failed");

    rc = bind(s_server, (struct sockaddr*)&sin6, sizeof(sin6));
    if (rc < 0) {
        perror("error bind webserver");
        exit(1);
    }

    rc = listen(s_server, BUFFSIZE);
    if (rc < 0) {
        perror("error listening");
        exit(1);
    }

    while(1) {
        s_client = accept(s_server, NULL, NULL);
        if (s_client < 0) {
            perror("error accepting client connection");
            continue;
        }

        run(s_client);
    }
}
