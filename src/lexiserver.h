#ifndef LEXISERVER_H
#define LEXISERVER_H

#include <sys/socket.h>
#include <arpa/inet.h>

// Definiciones de constantes
#define CONFIG_FILE "lexiserver.conf"
#define BUFFER_SIZE 1024

// Valores por defecto
#define DEFAULT_LPORT 1337
#define DEFAULT_WEB_ROOT "/tmp/www"
#define DEFAULT_LBUFSIZE 1024
#define DEFAULT_LEXISERVER "lexiserver-1.0.2"

// Declaraciones de funciones
void parse_config_file(int *LPORT, char *WEB_ROOT, int *LBUFSIZE, char *LEXISERVER);
void handle_request(int newsockfd, struct sockaddr_in client_addr, const char *WEB_ROOT, int LBUFSIZE);
void send_file(int sockfd, const char *filepath, int LBUFSIZE);
void send_error(int sockfd, int status_code, const char *error_page);
void list_directory(int sockfd, const char *dirpath);

extern const char *WEB_ROOT; // Agregar esta línea

#endif // LEXISERVER_H
