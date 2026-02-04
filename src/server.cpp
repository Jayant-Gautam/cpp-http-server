#include<iostream>
#include<string>
#include<unistd.h>
#include<netinet/in.h>
#include<vector>
#include "server.hpp"

void Server::run() {

    // socket creation
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "Socket created with fd: " << socket_fd << endl;
    if(socket_fd < 0){
        perror("Socket Error");
        return ;
    }

    // binding socket to ip add and port 
    // -> sockaddr_in is a struct that contains the address family, ip address and port number 
    sockaddr_in* address = new sockaddr_in();
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = in_addr_t{0X00000000};
    address->sin_port = htons(8080);

    int socket_bind = bind(socket_fd, (sockaddr*)address, sizeof(*address));
    cout << "Socket bind result: " << socket_bind << endl;
    if(socket_bind < 0){
        perror("Bind Error");
        return ;
    }

    // listen for incoming connections
    // listen(socket to be listened on, max number of connections allowed in the queue)
    int socket_listen = listen(socket_fd, 5);
    cout << "Socket listen result: " << socket_listen << endl;
    if(socket_listen < 0){
        perror("Listen Error");
        return ;
    }

    cout << "Server running" << endl << "http://localhost:8080" << endl;

    // creating seperate socket for each client connection
    while(true){
        int socket_client_fd = accept(socket_fd, NULL, NULL);
        cout << "Client connected with fd: "<< socket_client_fd << endl;
        if(socket_client_fd < 0){
            perror("Accept Error");
            continue;
        }

        char buffer[4096]; // as per the standard size of a page -> 4kb(4096 bytes) char size -> 1B

        // reading the request fromt the client
        read(socket_client_fd, buffer, sizeof(buffer));

        // printing the request recieved from the client
        cout << "Request: " << endl;
        for(auto it : buffer){
            cout << it;
        }
        cout << endl;

        // sending response to the client 
        string response = 
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 68\r\n"
            "\r\n"
            "<html><body><h1>Hello from C++ Server</h1></body></html>";
        write(socket_client_fd, response.c_str(), response.size());

        // closing the tcp connection with this->client so that server can listen to another client in the queue
        close(socket_client_fd);
    }
    return ;
}