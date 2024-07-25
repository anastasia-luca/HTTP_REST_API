#include <stdlib.h>     /* exit, atoi, malloc, free */
#include <stdio.h>
#include <unistd.h>     /* read, write, close */
#include <string.h>     /* memcpy, memset */
#include <sys/socket.h> /* socket, connect */
#include <netinet/in.h> /* struct sockaddr_in, struct sockaddr */
#include <netdb.h>      /* struct hostent, gethostbyname */
#include <arpa/inet.h>
#include <ctype.h> // for isdigit function
#include "helpers.h"
#include "requests.h"
#include "commands.h"
#include "parson.h"

// Define URL paths
#define REGISTER_URL "/api/v1/tema/auth/register"
#define LOGIN_URL "/api/v1/tema/auth/login"
#define ACCESS_URL "/api/v1/tema/library/access"
#define BOOKS_URL "/api/v1/tema/library/books"
#define LOGOUT_URL "/api/v1/tema/auth/logout"

#define CONTENT_TYPE "application/json"

#define HOST "34.246.184.49"
#define PORT 8080

char session_cookie[250] = {0};
char jwt_token[512] = {0};

void register_user(int sockfd) {
    char username[50], password[50];
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    // Open the connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Create JSON object
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    // Convert JSON object to string
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    
    // Create POST request message
    char *message = compute_post_request(HOST, REGISTER_URL, CONTENT_TYPE, serialized_string, NULL, 0, NULL);
    send_to_server(sockfd, message); // Send to server the request

    char *response = receive_from_server(sockfd);
    // Extract status code from server's response
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    if (status_code / 100 == 2) {
        printf("SUCCESS: Registration successful.\n");
    } else if (status_code / 100 == 4) {
        printf("ERROR: The username %s is taken!\n", username);
    }

    free(message);
    free(response);
    // Close the connection
    close_connection(sockfd);
}

void login_user(int sockfd) {
    // if (session_cookie[0] != '\0') {
    //     printf("ERROR: You are already logged in!\n");
    //     return; // Exit because a session is active
    // }

    char username[50], password[50];
    printf("username=");
    scanf("%s", username);
    printf("password=");
    scanf("%s", password);

    // Open the connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    // Create JSON object
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "username", username);
    json_object_set_string(root_object, "password", password);

    // Convert JSON object to string
    char *serialized_string = json_serialize_to_string_pretty(root_value);
    
    // Create POST request message
    char *message = compute_post_request(HOST, LOGIN_URL, CONTENT_TYPE, serialized_string, NULL, 0, NULL);
    send_to_server(sockfd, message); // Send to server the request

    char *response = receive_from_server(sockfd);
    // Extract status code from server's response
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    // Extract the session cookie from response's headers
    char *cookie_header = strstr(response, "Set-Cookie:");
    if (cookie_header != NULL) {
        sscanf(cookie_header, "Set-Cookie: %s", session_cookie);
        session_cookie[strlen(session_cookie) - 1] = '\0';
    }

    if (status_code / 100 == 2) {
        printf("SUCCESS: Logged in.\n");
    } else if (status_code / 100 == 4) {
        printf("ERROR: Credentials are not good!\n");
    }

    free(message);
    free(response);
    // Close the connection
    close_connection(sockfd);
}

void enter_library(int sockfd) {
    if (session_cookie[0] == '\0') {
        printf("ERROR: You are not logged in!\n");
        return; // Exit because no session is present
    }

    // Open the connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);
    
    // Store the session cookie
    char *cookies[1] = {session_cookie};
    char *message = compute_get_request(HOST, ACCESS_URL, NULL, cookies, 1, NULL);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    // Extract status code from server's response
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    // Reach the server's JSON response
    char* json_header = strstr(response, "\r\n\r\n");
    json_header += 4; // Move past the "\r\n\r\n"

    // Parse the JSON body
    JSON_Value *root_value = json_parse_string(json_header);
    JSON_Object *root_object = json_value_get_object(root_value);
    // Extract the JSON value
    const char *token = json_object_get_string(root_object, "token");
    if (token) {
        strncpy(jwt_token, token, sizeof(jwt_token) - 1);
        jwt_token[sizeof(jwt_token) - 1] = '\0';
    }

    if (status_code / 100 == 2) {
        printf("SUCCESS: Access granted.\n");
    } else {
        printf("ERROR: Access denied.\n");
    }

    free(message);
    free(response);
    json_value_free(root_value);
    close_connection(sockfd);
}

void get_books(int sockfd) {
    if (session_cookie[0] == '\0') {
        printf("ERROR: You are not logged in!\n");
        return; // Exit because no session is present
    }

    if (jwt_token[0] == '\0') {
        printf("ERROR: Unauthorized operation! Get access to library first.\n");
        return; // Exit because no token for accessing library exists
    }

    // Open the connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    char *cookies[1] = {session_cookie};
    char *message = compute_get_request(HOST, BOOKS_URL, NULL, cookies, 1, jwt_token);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    // Extract status code from server's response
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    // Reach the server's JSON response
    char* json_header = strstr(response, "\r\n\r\n");
    json_header += 4; // Move past the "\r\n\r\n"
    
    // Check the status code
    if (status_code / 100 == 2) {
        // Parse JSON and print book objects
        JSON_Value *root_value = json_parse_string(json_header);
        // Serialize the JSON value for pretty printing
        char *formatted_json = json_serialize_to_string_pretty(root_value);
        printf("%s\n", formatted_json);
        json_value_free(root_value);
    }
    else {
        printf("Failed to retrieve books.\n");
    }
    
    free(message);
    free(response);
    close_connection(sockfd);
}

void get_book(int sockfd) {
    if (session_cookie[0] == '\0') {
        printf("ERROR: You are not logged in!\n");
        return; // Exit because no session is present
    }

    if (jwt_token[0] == '\0') {
        printf("ERROR: Unauthorized operation! Get access to library first.\n");
        return;
    }

    // Open the connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    int bookId;
    printf("id=");
    scanf("%d", &bookId);

    char url[256];
    sprintf(url, "%s/%d", BOOKS_URL, bookId);

    char *cookies[1] = {session_cookie};
    char *message = compute_get_request(HOST, url, NULL, cookies, 1, jwt_token);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    char *json_header = strstr(response, "\r\n\r\n");
    json_header += 4;  // Skip the HTTP headers

    if (status_code / 100 == 2) {
            JSON_Value *root_value = json_parse_string(json_header);
            char *formatted_json = json_serialize_to_string_pretty(root_value);
            printf("%s\n", formatted_json);
            json_value_free(root_value);
    } else {
            printf("No book was found!\n");
    }

    free(message);
    free(response);
    close_connection(sockfd);
}

void add_book(int sockfd) {
    if (session_cookie[0] == '\0') {
        printf("ERROR: You are not logged in!\n");
        return; // Exit because no session is present
    }

    if (jwt_token[0] == '\0') {
        printf("ERROR: Unauthorized operation! Get access to library first.\n");
        return;
    }

    char title[100], author[100], genre[50], publisher[100], page_count_str[1000], dummy;
    int page_count;

    /* Remove the newline character from stdin buffer that is left
    by previous input operation (command add_book\n) */
    scanf("%c", &dummy);

    printf("title=");
    fgets(title, sizeof(title), stdin);
    title[strcspn(title, "\n")] = 0; // Remove the newline character

    printf("author=");
    fgets(author, sizeof(author), stdin);
    author[strcspn(author, "\n")] = 0;

    printf("genre=");
    fgets(genre, sizeof(genre), stdin);
    genre[strcspn(genre, "\n")] = 0;

    printf("publisher=");
    fgets(publisher, sizeof(publisher), stdin);
    publisher[strcspn(publisher, "\n")] = 0;

    printf("page_count=");
    fgets(page_count_str, sizeof(page_count_str), stdin);
    page_count_str[strcspn(page_count_str, "\n")] = 0;

    // If at least one field is incomplete throw error
    if (title[0] == '\0' || author[0] == '\0' ||
        genre[0] == '\0' || publisher[0] == '\0' ||
        page_count_str[0] == '\0') {
        printf("ERROR: Empty field. Failed to add the book.\n");
        return;
    }

    // Iterate through page input
    for (int i = 0; page_count_str[i] != '\0'; i++) {
        if (!isdigit(page_count_str[i])) { // Check if the character is not a digit
            printf("ERROR: Wrong type for page number. Please enter only numbers.\n");
            return;
        }
    }

    // Convert page count type from char to an integer
    page_count = atoi(page_count_str);

    // Create JSON object for the book
    JSON_Value *root_value = json_value_init_object();
    JSON_Object *root_object = json_value_get_object(root_value);
    json_object_set_string(root_object, "title", title);
    json_object_set_string(root_object, "author", author);
    json_object_set_string(root_object, "genre", genre);
    json_object_set_string(root_object, "publisher", publisher);
    json_object_set_number(root_object, "page_count", page_count);

    // Serialize the JSON object
    char *serialized_string = json_serialize_to_string_pretty(root_value);

    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    char *cookies[1] = {session_cookie};
    char *message = compute_post_request(HOST, BOOKS_URL, CONTENT_TYPE, serialized_string, cookies, 1, jwt_token);
    send_to_server(sockfd, message);

    json_free_serialized_string(serialized_string);
    json_value_free(root_value);

    char *response = receive_from_server(sockfd);
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    if (status_code / 100 == 2) {
        printf("SUCCESS: Book added successfully.\n");
    } else {
        printf("ERROR: Failed to add the book.\n");
    }

    free(message);
    free(response);
    close_connection(sockfd);
}

void delete_book(int sockfd) {
    if (session_cookie[0] == '\0') {
        printf("ERROR: You are not logged in!\n");
        return; // Exit because no session is present
    }

    if (jwt_token[0] == '\0') {
        printf("ERROR: Unauthorized operation! Get access to library first.\n");
        return;
    }

    // Open the connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    int bookId;
    printf("id=");
    scanf("%d", &bookId);

    char url[256];
    sprintf(url, "%s/%d", BOOKS_URL, bookId);

    char *cookies[1] = {session_cookie};
    char *message = compute_delete_request(HOST, url, cookies, 1, jwt_token);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    if (status_code / 100 == 2) {
        printf("SUCCESS: The book with id %d was deleted.\n", bookId);
    } else {
            printf("No book was found!\n");
    }

    free(message);
    free(response);
    close_connection(sockfd);
}

void logout_user(int sockfd) {
    if (session_cookie[0] == '\0') {
        printf("ERROR: You are not logged in!\n");
        return; // Exit because no session is present
    }

    // Open the connection
    sockfd = open_connection(HOST, PORT, AF_INET, SOCK_STREAM, 0);

    char *cookies[1] = {session_cookie};
    char *message = compute_get_request(HOST, LOGOUT_URL, NULL, cookies, 1, jwt_token);
    send_to_server(sockfd, message);

    char *response = receive_from_server(sockfd);
    int status_code;
    sscanf(response, "HTTP/1.1 %d", &status_code);

    if (status_code / 100 == 2) {
        printf("SUCCESS: Logged out.\n");
        session_cookie[0] = '\0';  // Clear the session cookie after successful logout
        jwt_token[0] = '\0';       // Clear the JWT token after successful logout
    } else {
        printf("ERROR: Couldn't log out.\n");
    }

    free(message);
    free(response);
    close_connection(sockfd);
}
