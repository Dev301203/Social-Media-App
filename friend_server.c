#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "friends.h"

#ifndef PORT
  #define PORT 50700
#endif

#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 12
#define MAX_BACKLOG 5
#define MAX_CONNECTIONS 1024
#define DELIM " \n"

typedef struct client {
    char name[MAX_NAME]; // name of the client
    // used for reading input
    char buf[INPUT_BUFFER_SIZE];
    int fd;
    int inbuf;
    int room;
    char *after;
    int where;
    // next client in linked list
    struct client *next;
} Client;

Client *clients;

User *user_list_ptr;

/* 
 * Print a formatted error message to stderr.
 */
void error(char *msg, int fd) {
    int num = write(fd, msg, strlen(msg) + 1);

    // checking if write worked as intended
    if (num == -1) {
        perror("write");
        exit(1);
    }
}


/*
 * Accept a connection. Note that a new file descriptor is created for
 * communication with the client. The initial socket descriptor is used
 * to accept connections, but the new socket is used to communicate.
 * Return the new client's file descriptor.
 */
void accept_client_connection(int listenfd) {
    struct sockaddr_in peer;
    unsigned int peer_len = sizeof(peer);
    peer.sin_family = AF_INET;

    int client_socket = accept(listenfd, (struct sockaddr *)&peer, &peer_len);
    if (client_socket < 0) {
        perror("accept");
        exit(1);
    }

    // initialize new client

    // dynamically allocate memory for the client
    Client *client = malloc(sizeof(Client));
    if (!client) {
    	perror("malloc");
    	exit(1);
    }

    client->fd = client_socket;
    client->inbuf = 0;
    client->room = 0;
    client->after = client->buf;
    client->where = 0;

    // adding the new client to the linked client list
    client->next = clients;
    clients = client;

    // empty client name
    for (int i = 0; i < MAX_NAME; i++){
    	client->name[i] = '\0';
    }

    // empty client buffer
    for (int i = 0; i < INPUT_BUFFER_SIZE; i++) {
        client->buf[i] = '\0';
    }

    // ask user for User name
    char *msg = "What is your user name?\r\n";
    int num =write(client->fd, msg, strlen(msg));

    // checking if write worked as intended
    if (num == -1) {
        perror("write");
        exit(1);
    }
}


/*
 * Search the first n characters of buf for a network newline (\r\n).
 * Return one plus the index of the '\n' of the first network newline,
 * or -1 if no network newline is found. The return value is the index into buf
 * where the current line ends.
 * Definitely do not use strchr or other string functions to search here. (Why not?)
 */
int find_network_newline(const char *buf, int n) {
    for (int i = 0; i < n; i++) {
        if (buf[i] == '\r') {
            if (buf[i+1] == '\n') {
                return i + 2;
            }
        }
    }
    return -1;
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv, int fd) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);    
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!", fd);
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    return cmd_argc;
}


/* 
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 */
int process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr, User *user, int fd) {
    User *user_list = *user_list_ptr;

    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        return -1;
    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
		char *buf = list_users(user_list);
		int num = write(fd, buf, strlen(buf) + 1);
        // checking if write worked as intended
        if (num == -1) {
            perror("write");
            exit(1);
        }
		free(buf);
    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 2) {
        char buf[INPUT_BUFFER_SIZE];
        char friend_buf[INPUT_BUFFER_SIZE];
        switch (make_friends(user->name, cmd_argv[1], user_list)) {
            case 0:
            	strcpy(buf, "You are now friends with ");
            	strcat(buf, cmd_argv[1]);
            	strcat(buf, "\r\n");
            	int num = write(fd, buf, strlen(buf));
                // checking if write worked as intended
                if (num == -1) {
                    perror("write");
                    exit(1);
                }
                // printing out message for all instances of the user that was friended
                strcpy(friend_buf, "You have been friended by ");
                strcat(friend_buf, user->name);
                strcat(friend_buf, "\r\n");
                Client *curr = clients;
                while (curr != NULL) {
                    if (strcmp(curr->name, cmd_argv[1]) == 0) {
                        int num = write(curr->fd, friend_buf, strlen(friend_buf));
                        // checking if write worked as intended
                        if (num == -1) {
                            perror("write");
                            exit(1);
                        }
                    }
                    curr = curr->next;
                }
            	break;
            case 1:
                error("You are already friends.\r\n", fd);
                break;
            case 2:
                error("At least one of you entered has the max number of friends\r\n", fd);
                break;
            case 3:
                error("You can't friend yourself\r\n", fd);
                break;
            case 4:
                error("The user you entered does not exist\r\n", fd);
                break;
        }
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 2; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents = malloc(space_needed);
        if (contents == NULL) {
            free(contents);
            perror("malloc");
            exit(1);
        }

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[2]);
        for (int i = 3; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }

        User *author = user;;
        User *target = find_user(cmd_argv[1], user_list);
        char buf[INPUT_BUFFER_SIZE];
        switch (make_post(author, target, contents)) {
            case 0:
                // printing out post for all instances of the user to whom the post was sent
                strcpy(buf, "From ");
                strcat(buf, author->name);
                strcat(buf, ": ");
                strcat(buf, target->first_post->contents);
                strcat(buf, "\r\n");
                Client *curr = clients;
                while (curr != NULL) {
                    if (strcmp(curr->name, target->name) == 0) {
                        int num = write(curr->fd, buf, strlen(buf));
                        // checking if write worked as intended
                        if (num == -1) {
                            perror("write");
                            exit(1);
                        }
                    }
                    curr = curr->next;
                }
                break;
            case 1:
                error("You can only post to your friends\r\n", fd);
                break;
            case 2:
                error("The user you want to post to does not exist\r\n", fd);
                break;
        }
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
        char *buf = print_user(user);
        if (strcmp(buf, "") == 0) {
            error("User not found\r\n", fd);
        }
        int num = write(fd, buf, strlen(buf) + 1);
        // checking if write worked as intended
        if (num == -1) {
            perror("write");
            exit(1);
        }
        free(buf);
    } else {
        error("Incorrect syntax\r\n", fd);
    }
    return 0;
}


/*
 * Read and process buffered client message from client fds.
 */
void read_from_client(Client *client) {
    // This part of the code was taken from lab11
    // Receive messages
    int nbytes = read(client->fd, client->after, client->room);
    
    // checking if the read worked as intended
    if (nbytes == -1) {
        perror("read");
        exit(1);
    }
    // update inbuf (how many bytes were just added?)
    client->inbuf += nbytes;

    // where is now the index into buf immediately after 
    // the first network newline
    client->where = find_network_newline(client->buf, client->inbuf);
    // the condition below calls find_network_newline
    // to determine if a full line has been read from the client.
    if (client->where > 0) {

        // Output the full line, not including the "\r\n",
        // using print statement below.
        client->buf[client->where - 1] = '\0';
        client->buf[client->where - 2] = '\0';

        // if client does not have name, either initialise client or search for client
        if (client->name[0] == '\0') {
            
            // check if client is already in the User's list
            User *user = user_list_ptr;
            while (user != NULL) {
                if (strcmp(user->name, client->buf) == 0) {
                    break;
                }
                user = user->next;
            }

            // client found in list of User's
            if (user != NULL) {

                // copy clients name
                strncpy(client->name, client->buf, sizeof(client->name) - 1);

                // welcome message
                char *msg = "Welcome back.\r\n";
                int num = write(client->fd, msg, strlen(msg));
                // checking if write worked as intended
                if (num == -1) {
                    perror("write");
                    exit(1);
                }
            }

            // initialising User name
            else {

                // if the name is too long, then truncate it
                if (strlen(client->buf) > MAX_NAME - 1) {
                    char *msg = "Username too long, truncated to 31 chars.\r\n";
                    int num = write(client->fd, msg, strlen(msg));
                    // checking if write worked as intended
                    if (num == -1) {
                        perror("write");
                        exit(1);
                    }
                }

                // otherwise, simply welcome user
                else {
                    char *msg = "Welcome.\r\n";
                    int num = write(client->fd, msg, strlen(msg));
                    // checking if write worked as intended
                    if (num == -1) {
                        perror("write");
                        exit(1);
                    }
                }

                // updating client info
                strncpy(client->name, client->buf, sizeof(client->name) - 1);

                // create the new user
                create_user(client->name, &user_list_ptr);
            }

            // ask user for commands
            char *msg = "Go ahead and enter user commands>\r\n";
            int num = write(client->fd, msg, strlen(msg));
            // checking if write worked as intended
            if (num == -1) {
                perror("write");
                exit(1);
            }
        }

        // if client has a name, find the user and call process_args on the tokenized command
        else {

            // tokenize input
            char *cmd_argv[INPUT_ARG_MAX_NUM];
            int cmd_argc = tokenize(client->buf, cmd_argv, client->fd);

            User *user = user_list_ptr;
            while (user != NULL) {
                if (strcmp(user->name, client->name) == 0) {
                    break;
                }
                user = user->next;
            }

            // process commands. if quit, then remove client from clients list
            if (cmd_argc > 0 && process_args(cmd_argc, cmd_argv, &user_list_ptr,
                    user, client->fd) == -1) {

                // iterating over client linked list to find the client
                Client *curr = clients;
                while (curr != NULL) {
                    if (curr == client) {
                        break;
                    }
                    curr = curr->next;
                }

                // removing client from client linked list by make the previous client linked to the next client of deleted client
                Client *prev = clients;
                if (prev == curr) {
                    clients = curr->next;
                } else {
                    while (prev->next != curr) {
                        prev = prev->next;
                    }
                    prev->next = curr->next;
                }

                // freeing dynamically allocated memory
                free(client);
            }
        }

        // update inbuf and remove the full line from the buffer
        client->inbuf -= client->where;

        for (int i = 0; i < client->where; i++) {
            client->buf[i] = '\0';
        }

        // You want to move the stuff after the full line to the beginning
        // of the buffer.
        memmove(&(client->buf), &(client->buf[client->where]), client->inbuf);

    }
    // update after and room, in preparation for the next read.
    client->after = client->buf + client->inbuf;
    client->room = INPUT_BUFFER_SIZE - client->inbuf;
}


int main() {

    // list of clients whose head is pointed to by *user_list_ptr
    clients = NULL;

    // list of User's whose head is pointed to by *user_list_ptr
    user_list_ptr = NULL;

    // This part of the code was taken from lab10
    // Create the socket FD.
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("server: socket");
        exit(1);
    }

    // Set information about the port (and IP) we want to be connected to.
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    server.sin_addr.s_addr = INADDR_ANY;

    // This sets an option on the socket so that its port can be reused right
    // away. Since you are likely to run, stop, edit, compile and rerun your
    // server fairly quickly, this will mean you can reuse the same port.
    int on = 1;
    int status = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR,
                            (const char *) &on, sizeof(on));
    if (status == -1) {
        perror("setsockopt -- REUSEADDR");
    }

    // This should always be zero. On some systems, it won't error if you
    // forget, but on others, you'll get mysterious errors. So zero it.
    memset(&server.sin_zero, 0, 8);

    // Bind the selected port to the socket.
    if (bind(sock_fd, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("server: bind");
        close(sock_fd);
        exit(1);
    }

    // Announce willingness to accept connections on this socket.
    if (listen(sock_fd, MAX_BACKLOG) < 0) {
        perror("server: listen");
        close(sock_fd);
        exit(1);
    }

    Client *client;

    while (1) {
        // The client accept - message accept loop. First, we prepare to listen to multiple
        // file descriptors by initializing a set of file descriptors.
        int max_fd = sock_fd;
        fd_set all_fds;
        FD_ZERO(&all_fds);
        FD_SET(sock_fd, &all_fds);

        // initialize fd's of all clients and find max_fd
        client = clients;
        while (client != NULL) {
            FD_SET(client->fd, &all_fds);
            if (client->fd > max_fd)
                max_fd = client->fd;
            client = client->next;
        }

        // waiting for activity on any fd 
        if (select(max_fd + 1, &all_fds, NULL, NULL, NULL) == -1) {
            perror("server: select");
            exit(1);
        }

        // check fds of clients
        client = clients;
        while (client != NULL) {
            if (FD_ISSET(client->fd, &all_fds)) {

                // if activity detected from client, process input
                read_from_client(client);
            }
            client = client->next;
        }

        // activity on server socket means new client connection
		if (FD_ISSET(sock_fd, &all_fds)){
			accept_client_connection(sock_fd);
		}
    }
}