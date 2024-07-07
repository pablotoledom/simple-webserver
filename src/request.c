#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include "lexiserver.h"

const char *WEB_ROOT = DEFAULT_WEB_ROOT; // Definir la variable global

void send_error(int sockfd, int status_code, const char *error_page) {
    char error_message[256];
    snprintf(error_message, sizeof(error_message), "HTTP/1.0 %d ERROR\r\n\r\n", status_code);
    write(sockfd, error_message, strlen(error_message));

    char buffer[BUFFER_SIZE];
    size_t bytes_read;

    FILE *file = fopen(error_page, "r");
    if (file == NULL) {
        perror("ERROR OPENING ERROR FILE");
        return;
    }

    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (write(sockfd, buffer, bytes_read) < 0) {
            perror("ERROR SENDING ERROR FILE TO CLIENT");
            fclose(file);
            return;
        }
    }

    fclose(file);
}

void get_parent_path(const char *path, char *parent_path) {
    // Copia el path a parent_path
    strncpy(parent_path, path, BUFFER_SIZE);
    // Elimina cualquier barra diagonal al final del path
    size_t len = strlen(parent_path);
    if (len > 0 && parent_path[len - 1] == '/') {
        parent_path[len - 1] = '\0';
    }
    // Encuentra la última barra diagonal y corta el path allí
    char *last_slash = strrchr(parent_path, '/');
    if (last_slash != NULL) {
        *last_slash = '\0';
    }
    // Si parent_path quedó vacío, ponlo como "/"
    if (strlen(parent_path) == 0) {
        strcpy(parent_path, "/");
    }
}

void list_directory(int sockfd, const char *dirpath) {
    DIR *d;
    struct dirent *dir;
    char buffer[BUFFER_SIZE];
    size_t buffer_remaining;
    int written;

    d = opendir(dirpath);
    if (d == NULL) {
        perror("webserver (opendir)");
        send_error(sockfd, 500, "500.html");
        return;
    }

    // Eliminar la parte de la ruta base del servidor de dirpath
    const char *relative_path = dirpath + strlen(WEB_ROOT);

    char response_headers[] = "HTTP/1.0 200 OK\r\n"
                              "Server: Lexiserver 1.0.2\r\n"
                              "Content-type: text/html\r\n\r\n";
    write(sockfd, response_headers, strlen(response_headers));

    buffer_remaining = sizeof(buffer);
    written = snprintf(buffer, buffer_remaining, "<html><head><title>Index of %s</title></head><body><h1>Index of %s</h1><ul>", relative_path, relative_path);
    if (written < 0 || written >= buffer_remaining) {
        send_error(sockfd, 500, "500.html");
        closedir(d);
        return;
    }
    write(sockfd, buffer, strlen(buffer));

    // Agregar un enlace para "subir un nivel" si no estamos en el directorio raíz
    if (strcmp(relative_path, "/") != 0) {
        char parent_path[BUFFER_SIZE];
        get_parent_path(relative_path, parent_path);
        // Asegurarse de que parent_path no tenga '//' al inicio
        if (parent_path[0] == '/' && parent_path[1] == '/') {
            memmove(parent_path, parent_path + 1, strlen(parent_path));
        }
        // Agregar '/' al final si no es el directorio raíz
        if (strcmp(parent_path, "/") != 0) {
            strncat(parent_path, "/", BUFFER_SIZE - strlen(parent_path) - 1);
        }
        buffer_remaining = sizeof(buffer);
        written = snprintf(buffer, buffer_remaining, "<li><a href=\"%s\">.. (up one level)</a></li>", parent_path);
        if (written < 0 || written >= buffer_remaining) {
            send_error(sockfd, 500, "500.html");
            closedir(d);
            return;
        }
        write(sockfd, buffer, strlen(buffer));
    }

    while ((dir = readdir(d)) != NULL) {
        if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0)
            continue;

        struct stat path_stat;
        char full_path[BUFFER_SIZE];
        snprintf(full_path, sizeof(full_path), "%s/%s", dirpath, dir->d_name);
        stat(full_path, &path_stat);

        if (S_ISDIR(path_stat.st_mode)) {
            buffer_remaining = sizeof(buffer);
            written = snprintf(buffer, buffer_remaining, "<li><a href=\"%s/\">%s/</a></li>", dir->d_name, dir->d_name);
        } else {
            buffer_remaining = sizeof(buffer);
            written = snprintf(buffer, buffer_remaining, "<li><a href=\"%s\">%s</a></li>", dir->d_name, dir->d_name);
        }
        if (written < 0 || written >= buffer_remaining) {
            send_error(sockfd, 500, "500.html");
            closedir(d);
            return;
        }
        write(sockfd, buffer, strlen(buffer));
    }

    buffer_remaining = sizeof(buffer);
    written = snprintf(buffer, buffer_remaining, "</ul></body></html>");
    if (written < 0 || written >= buffer_remaining) {
        send_error(sockfd, 500, "500.html");
        closedir(d);
        return;
    }
    write(sockfd, buffer, strlen(buffer));

    closedir(d);
}
