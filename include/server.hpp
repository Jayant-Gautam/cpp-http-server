#pragma once
#include <sys/types.h>
#include <unordered_map>
#include <string>
using namespace std;
#include "client_type.hpp"
#include "router.hpp"
#include "multithreading.hpp"


class Server {
    ThreadPool threadPool{10}; // creating a thread pool with 4 worker threads to handle client requests concurrently
    mutex clients_mutex;
    int port;
    Router router;
    unordered_map<int,Client> clients;
    fd_set master_set, read_set, write_set; // set of file discriptors to be monitored for incoming connections and data to be read
    void handle_client_read(int client_socket_fd, int epoll_fd);
    void handle_client_write(int client_socket_fd, int epoll_fd);
    void reset_client(Client &client);
public:
    void run();
    Server(int port);
};
