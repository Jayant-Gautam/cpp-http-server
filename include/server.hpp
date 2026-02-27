#pragma once
#include <sys/types.h>
#include <unordered_map>
#include <string>
using namespace std;

struct Client {
    std::string read_buffer;
    std::string write_buffer;
    bool header_complete = false;
};

class Server {
    unordered_map<int,Client> clients;
    fd_set master_set, read_set; // set of file discriptors to be monitored for incoming connections and data to be read
    void handle_client(int client_socket_fd);
public:
    void run();
};
