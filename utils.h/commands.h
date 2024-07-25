#ifndef COMMANDS_H
#define COMMANDS_H

void register_user(int sockfd);
void login_user(int sockfd);
void enter_library(int sockfd);
void get_books(int sockfd);
void get_book(int sockfd);
void add_book(int sockfd);
void delete_book(int sockfd);
void logout_user(int sockfd);

#endif