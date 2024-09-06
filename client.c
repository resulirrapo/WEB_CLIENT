#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "parson.h"
#include "parson.c"
#include "helpers.h"
#include "requests.h"

// 
void read_input(char *input, const char *prompt) {
    printf("%s", prompt);
    fgets(input, LINELEN, stdin);
    input[strcspn(input, "\n")] = 0;
}

void check_cookie_and_jwt(char *cookie, char *JWT) {
    if (!cookie) {
        printf("Client not logged in\n");
        exit(1);
    }
    if (!JWT) {
        printf("Client did not request library access\n");
        exit(1);
    }
}

char* send_request_and_receive_response(char *message, int sockfd) {
    send_to_server(sockfd, message);
    return receive_from_server(sockfd);
}

// we send the register request to the server
void register_client(char *host, char *registerURL, char *contentype, int sockfd) {
    char username[LINELEN], password[LINELEN];
    read_input(username, "username=");
    read_input(password, "password=");

    // add the username and password to a JSON object
    JSON_Value *root_credentials = json_value_init_object();
    JSON_Object *credentials = json_value_get_object(root_credentials);
    json_object_set_string(credentials, "username", username);
    json_object_set_string(credentials, "password", password);
    char *json_string = json_serialize_to_string(root_credentials);

    char *message = compute_post_request(host, registerURL, contentype, (char **)&json_string, NULL, 0, NULL);
    // send the request to the server and receive the response
    char *response = send_request_and_receive_response(message, sockfd);
    
    // treats the response from the server
    strtok(response, " ");
    char *exit_code = strtok(NULL, " ");
    if (strcmp(exit_code, "201") == 0) 
        printf("Success\n");
    else 
        printf("Username already taken\n");

    free(json_string);
    json_value_free(root_credentials);
}

// we send the login request to the server
char* login_client(char *host, char *loginURL, char *contentype, int sockfd) {
    char username[LINELEN], password[LINELEN];

    // read credentials from the user
    read_input(username, "username=");
    read_input(password, "password=");

    if (strpbrk(username, " ")) {
        printf("Invalid username\n");
        return NULL;
    }

    // add the username and password to a JSON object
    JSON_Value *root_credentials = json_value_init_object();
    JSON_Object *credentials = json_value_get_object(root_credentials);
    json_object_set_string(credentials, "username", username);
    json_object_set_string(credentials, "password", password);
    char *json_string = json_serialize_to_string(root_credentials);

    char *message = compute_post_request(host, loginURL, contentype, (char **)&json_string, NULL, 0, NULL);

    // send the request to the server and receive the response
    char *response = send_request_and_receive_response(message, sockfd);

    // checks the response and extracts the cookie
    strtok(response, " ");
    char *exit_code = strtok(NULL, " ");
    char *cookie = NULL;
    if (strcmp(exit_code, "200") == 0) {
        while ((exit_code = strtok(NULL, " "))) {
            if (strncmp(exit_code, "connect", 6) == 0) {
                cookie = malloc(LINELEN);
                strncpy(cookie, exit_code, LINELEN);
                cookie = strtok(cookie, ";");
                break;
            }
        }
        printf("Success\n");
    } else {
        printf("Credentials don't match\n");
    }

    free(json_string);
    json_value_free(root_credentials);
    return cookie;
}

// sends enter_library request
char* enter_library(char *cookie, char *host, char *libraryURL, int sockfd) {
    if (!cookie) {
        printf("Log in to use this feature\n");
        return NULL;
    }

    char *message = compute_get_request(host, libraryURL, NULL, &cookie, 1, NULL);
    char *response = send_request_and_receive_response(message, sockfd);

    char *json_string = basic_extract_json_response(response);
    strtok(json_string, "\"");

    // extract JWT from the response
    for (int i = 0; i < 2; i++) {
        strtok(NULL, "\"");
    }
    char *JWT = strtok(NULL, "\"");
    printf("Success\n");
    return JWT;
}

// sends get_books request
void get_books(char *cookie, char *JWT, char *host, char *booksURL, int sockfd) {
    // check credentials and prepare the HTTP GET request
    check_cookie_and_jwt(cookie, JWT);    
    char *message = compute_get_request(host, booksURL, NULL, &cookie, 1, JWT);
    
    // send the request and receive the response
    char *response = send_request_and_receive_response(message, sockfd);

    // extract JSON from response and print if it's not null
    char *result = basic_extract_json_response(response);
    if (result && strlen(result) > 0) {
        printf("Books retrieved successfully:\n%s\n", result-1);
    } else {
        printf("No books found or failed to extract JSON.\n");
    }
}

// sends get_book request
void get_book(char *cookie, char *JWT, char *host, char *bookURL, int sockfd) {
    // check credentials and prepare the HTTP GET request
    check_cookie_and_jwt(cookie, JWT);

    char id[LINELEN], URL[LINELEN];

    // get the book id from the user and add it to the URL
    read_input(id, "id=");
    snprintf(URL, LINELEN, "%s%s", bookURL, id);

    // creates the get_request and sends it to the server
    char *message = compute_get_request(host, URL, NULL, &cookie, 1, JWT);
    char *response = send_request_and_receive_response(message, sockfd);
    char *result = basic_extract_json_response(response);

    if (result) {
        printf("%s\n", result);
    } else {
        printf("No such book\n");
    }
}

// sends add_book request
void add_book(char *cookie, char *JWT, char *host, char *booksURL, char *contentype, int sockfd) {
    // check credentials and prepare the HTTP POST request
    check_cookie_and_jwt(cookie, JWT);
    char title[LINELEN], author[LINELEN], genre[LINELEN], publisher[LINELEN], page_count[LINELEN];

    // read book details from the user
    read_input(title, "title=");
    read_input(author, "author=");
    read_input(genre, "genre=");
    read_input(publisher, "publisher=");
    read_input(page_count, "page_count=");

    if (strlen(title) == 0 || strlen(page_count) == 0) {
        printf("Error: All fields must be filled\n");
        return;
    }

    for (int i = 0; i < strlen(page_count); i++) {
        if (!isdigit(page_count[i])) {
            printf("Error: Page count must be a number\n");
            return;
        }
    }

    // add info to a JSON and send reuqest
    JSON_Value *root_book = json_value_init_object();
    JSON_Object *bookinfo = json_value_get_object(root_book);
    json_object_set_string(bookinfo, "title", title);
    json_object_set_string(bookinfo, "author", author);
    json_object_set_string(bookinfo, "genre", genre);
    json_object_set_string(bookinfo, "publisher", publisher);
    json_object_set_string(bookinfo, "page_count", page_count);
    char *json_string = json_serialize_to_string(root_book);

    char *message = compute_post_request(host, booksURL, contentype, &json_string, NULL, 0, JWT);
    char *response = send_request_and_receive_response(message, sockfd);

    strtok(response, " ");
    char *exit_code = strtok(NULL, " ");
    if (strcmp(exit_code, "200") == 0) {
        printf("Book added successfully\n");
    } else {
        printf("Failed to add book: Server returned status %s\n", exit_code);
    }

    free(json_string);
    json_value_free(root_book);
}


// sends delete_book request
void delete_book(char *cookie, char *JWT, char *host, char *bookURL, int sockfd) {\
    // check credentials and prepare the HTTP DELETE request
    check_cookie_and_jwt(cookie, JWT);
    char id[LINELEN], URL[LINELEN];
    // get the book id from the user and add it to the URL
    read_input(id, "id=");
    snprintf(URL, LINELEN, "%s%s", bookURL, id);

    char *message = compute_delete_request(host, URL, &cookie, 1, JWT);
    char *response = send_request_and_receive_response(message, sockfd);

    // check the response and print the result
    strtok(response, " ");
    char *exit_code = strtok(NULL, " ");
    if (strcmp(exit_code, "200") == 0) {
        printf("Success\n");
    } else {
        printf("No such book\n");
    }
}

// sends logout request
void logout(char *cookie, char *host, char *logoutURL, int sockfd) {
    if (!cookie) {
        printf("Client not logged in\n");
        return;
    }

    char *message = compute_get_request(host, logoutURL, NULL, &cookie, 1, NULL);
    send_request_and_receive_response(message, sockfd);
    cookie = NULL;
    printf("Log out successful\n");
}

int main(int argc, char *argv[]) {

    u_int16_t port = 8080;
    char ipserver[LINELEN] = "34.246.184.49";

    char host[LINELEN] = "Host: 34.246.184.49";
    char contentype[LINELEN] = "Content-Type: application/json";
    char registerURL[LINELEN] = "/api/v1/tema/auth/register";
    char loginURL[LINELEN] = "/api/v1/tema/auth/login";
    char libraryURL[LINELEN] = "/api/v1/tema/library/access";
    char booksURL[LINELEN] = "/api/v1/tema/library/books";
    char bookURL[LINELEN] = "/api/v1/tema/library/books/";
    char logoutURL[LINELEN] = "/api/v1/tema/auth/logout";
    char command[COMMANDSIZE];
    char *cookie = NULL;
    char *JWT = NULL;

    while (1) {
        int sockfd = open_connection(ipserver, port, AF_INET, SOCK_STREAM, 0);
        read_input(command, "");

        if (strcmp(command, "register") == 0) register_client(host, registerURL, contentype, sockfd);
        if (strcmp(command, "login") == 0) cookie = login_client(host, loginURL, contentype, sockfd);
        if (strcmp(command, "enter_library") == 0) JWT = enter_library(cookie, host, libraryURL, sockfd);
        if (strcmp(command, "get_books") == 0) get_books(cookie, JWT, host, booksURL, sockfd);
        if (strcmp(command, "add_book") == 0) add_book(cookie, JWT, host, booksURL, contentype, sockfd);
        if (strcmp(command, "get_book") == 0) get_book(cookie, JWT, host, bookURL, sockfd);
        if (strcmp(command, "delete_book") == 0) delete_book(cookie, JWT, host, bookURL, sockfd);
        if (strcmp(command, "logout") == 0) {
            logout(cookie, host, logoutURL, sockfd);
            // if logout you cant access the library anymore
            cookie = NULL;
            JWT = NULL;
            }

        if (strcmp(command, "exit") == 0) break;

        close_connection(sockfd);
    }

    return 0;
}
