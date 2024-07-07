#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include "lexiserver.h"

void handle_request(int newsockfd, struct sockaddr_in client_addr, const char *WEB_ROOT, int LBUFSIZE) {
    char buffer[LBUFSIZE];
    int valread = read(newsockfd, buffer, LBUFSIZE);
    if (valread < 0) {
        perror("webserver (read)");
        close(newsockfd);
        return;
    }

    char method[LBUFSIZE], uri[LBUFSIZE], version[LBUFSIZE];
    sscanf(buffer, "%s %s %s", method, uri, version);
    printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), method, uri, version);

    char filepath[LBUFSIZE];
    snprintf(filepath, sizeof(filepath), "%s%s", WEB_ROOT, uri);
    struct stat path_stat;
    stat(filepath, &path_stat);

    if (S_ISDIR(path_stat.st_mode)) {
        char index_path[LBUFSIZE];
        snprintf(index_path, sizeof(index_path), "%s/index.html", filepath);
        if (access(index_path, F_OK) != -1) {
            send_file(newsockfd, index_path, LBUFSIZE);
        } else {
            list_directory(newsockfd, filepath);
        }
    } else {
        int filefd = open(filepath, O_RDONLY);
        if (filefd < 0) {
            perror("webserver (open)");
            send_error(newsockfd, 404, "404.html");
        } else {
            send_file(newsockfd, filepath, LBUFSIZE);
            close(filefd);
        }
    }

    close(newsockfd);
}

void send_file(int sockfd, const char *filepath, int LBUFSIZE) {
    char buffer[LBUFSIZE];
    ssize_t file_size;
    int valwrite;

    int filefd = open(filepath, O_RDONLY);
    if (filefd < 0) {
        perror("webserver (open file)");
        return;
    }

    char response_headers[] = "HTTP/1.0 200 OK\r\n"
                              "Server: Lexiserver 1.0.2\r\n"
                              "Content-type: text/html\r\n\r\n";
    valwrite = write(sockfd, response_headers, strlen(response_headers));
    if (valwrite < 0) {
        perror("webserver (write headers)");
        close(filefd);
        return;
    }

    while ((file_size = read(filefd, buffer, LBUFSIZE)) > 0) {
        valwrite = write(sockfd, buffer, file_size);
        if (valwrite < 0) {
            perror("webserver (write file)");
            close(filefd);
            return;
        }
    }

    close(filefd);
}
