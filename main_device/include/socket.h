#ifndef SOCKET_H
#define SOCKET_H

#define PORT 12345

int setup_server_socket();
int accept_client_connection(int server_fd);

#endif // SOCKET_H
