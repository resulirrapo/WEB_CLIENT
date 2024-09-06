 #include <stdlib.h> /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"

void add_cookies_to_request(char *message, char **cookies, int cookies_count) {
    if (cookies != NULL) {
        char *line = calloc(LINELEN, sizeof(char));
        sprintf(line, "Cookie: %s", *cookies);
        compute_message(message, line);
        free(line);
    }
}

void add_jwt_to_request(char *message, char *JWT) {
    if (JWT != NULL) {
        char *line = calloc(LINELEN, sizeof(char));
        sprintf(line, "Authorization: Bearer %s", JWT);
        compute_message(message, line);
        free(line);
    }
}

// Compute the message corresponding to a GET request
char *compute_get_request(char *host, char *url, char *query_params,
                          char **cookies, int cookies_count, char* JWT) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Write the method name, URL, request params (if any), and protocol type
    if (query_params != NULL) {
        sprintf(line, "GET %s?%s HTTP/1.1", url, query_params);
    } else {
        sprintf(line, "GET %s HTTP/1.1", url);
    }
    compute_message(message, line);

    // Add the host
    compute_message(message, host);

    // Add cookies and JWT
    add_cookies_to_request(message, cookies, cookies_count);
    add_jwt_to_request(message, JWT);

    compute_message(message, "");
    return message;
}

// Compute the message corresponding to a POST request
char *compute_post_request(char *host, char *url, char *content_type, char **body_data,
                           char **cookies, int cookies_count, char* JWT) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));
    char *length = calloc(LINELEN, sizeof(char));

    // Write the method name, URL, and protocol type
    sprintf(line, "POST %s HTTP/1.1", url);
    compute_message(message, line);

    // Add the host
    compute_message(message, host);

    // Add headers
    compute_message(message, content_type);
    sprintf(length, "Content-Length: %ld", strlen(*body_data));
    compute_message(message, length);
    
    // Add cookies and JWT
    add_cookies_to_request(message, cookies, cookies_count);
    add_jwt_to_request(message, JWT);

    compute_message(message, "");

    // Add JSON data
    compute_message(message, *body_data);

    return message;
}

// Compute the message corresponding to a DELETE request
char *compute_delete_request(char *host, char *url, char **cookies, int cookies_count, char* JWT) {
    char *message = calloc(BUFLEN, sizeof(char));
    char *line = calloc(LINELEN, sizeof(char));

    // Write the method name, URL, and protocol type
    sprintf(line, "DELETE %s HTTP/1.1", url);
    compute_message(message, line);

    // Add the host
    compute_message(message, host);

    // Add cookies and/or JWT
    add_cookies_to_request(message, cookies, cookies_count);
    add_jwt_to_request(message, JWT);

    compute_message(message, "");
    return message;
}