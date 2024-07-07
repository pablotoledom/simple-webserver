#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "lexiserver.h"

int main() {
    int LPORT = DEFAULT_LPORT, LBUFSIZE = DEFAULT_LBUFSIZE;
    char WEB_ROOT[256] = DEFAULT_WEB_ROOT, LEXISERVER[256] = DEFAULT_LEXISERVER;
    parse_config_file(&LPORT, WEB_ROOT, &LBUFSIZE, LEXISERVER);

    printf("* Configuration Loaded:\n* PORT=%d\n* WEB_ROOT=%s\n* BUFFER SIZE=%d\n* LEXISERVER=%s\n", LPORT, WEB_ROOT, LBUFSIZE, LEXISERVER);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("webserver (socket)");
        return 1;
    }
    printf("* Socket Created [ OK ]\n* Server Listening\n* Lexiserver 1.0.2 https://alexia.lat/docs/lexiserver\n");

    struct sockaddr_in host_addr, client_addr;
    socklen_t host_addrlen = sizeof(host_addr);
    socklen_t client_addrlen = sizeof(client_addr);

    memset(&host_addr, 0, sizeof(host_addr));
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(LPORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        perror("webserver (bind)");
        return 1;
    }

    printf("* Bound Socket to Address [ OK ]\n");

    if (listen(sockfd, SOMAXCONN) != 0) {
        perror("webserver (listen)");
        return 1;
    }

    printf("* Server Listening Connections on Port %d\n", LPORT);

    for (;;) {
        int newsockfd = accept(sockfd, (struct sockaddr *)&client_addr, &client_addrlen);
        if (newsockfd < 0) {
            perror("webserver (accept)");
            continue;
        }
        printf("connection accepted\n");

        handle_request(newsockfd, client_addr, WEB_ROOT, LBUFSIZE);
    }

    close(sockfd);
    return 0;
}
