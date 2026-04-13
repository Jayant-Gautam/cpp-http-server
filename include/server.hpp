#pragma once
#include <sys/types.h>
#include <unordered_map>
#include <string>
using namespace std;
#include "client_type.hpp"


class Server {
    unordered_map<int,Client> clients;
    fd_set master_set, read_set, write_set; // set of file discriptors to be monitored for incoming connections and data to be read
    void handle_client_read(int client_socket_fd, int epoll_fd);
    void handle_client_write(int client_socket_fd, int epoll_fd);
    void reset_client(Client &client);
public:
    void run();
};
