#include <stdio.h>      /* printf, sprintf */
#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include "helpers.h"
#include "requests.h"
#include "commands.h"

// Define command names
#define REGISTER_COMMAND "register"
#define LOGIN_COMMAND "login"
#define ENTER_LIBRARY_COMMAND "enter_library"
#define GET_BOOKS_COMMAND "get_books"
#define GET_BOOK_COMMAND "get_book"
#define ADD_BOOK_COMMAND "add_book"
#define DELETE_BOOK_COMMAND "delete_book"
#define LOGOUT_COMMAND "logout"
#define EXIT_COMMAND "exit"

#define HOST "34.246.184.49"
#define PORT 8080

int main(int argc, char *argv[])
{
    int sockfd;
    char command[20];

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    while (1) {
        scanf("%s", command);
        if (strcmp(command, "exit") == 0) {
            break;
        } else if (strcmp(command, "register") == 0) {
            register_user(sockfd);
        } else if (strcmp(command, "login") == 0) {
            login_user(sockfd);
        } else if (strcmp(command, "enter_library") == 0) {
            enter_library(sockfd);
        } else if (strcmp(command, "get_books") == 0) {
            get_books(sockfd);
        } else if (strcmp(command, "get_book") == 0) {
            get_book(sockfd);
        } else if (strcmp(command, "add_book") == 0) {
            add_book(sockfd);
        } else if (strcmp(command, "delete_book") == 0) {
            delete_book(sockfd);
        } else if (strcmp(command, "logout") == 0) {
            logout_user(sockfd);
        } else {
            printf("Invalid command!\n");
        }
    }
    
    close_connection(sockfd);
    return 0;
}
