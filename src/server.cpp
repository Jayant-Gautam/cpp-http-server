#include <iostream>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <vector>
#include "server.hpp"
#include "response.hpp"
#include "parseHTTP.hpp"
#include "mapRoute.hpp"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

void Server::run()
{

    // socket creation
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    cout << "Socket created with fd: " << socket_fd << endl;
    if (socket_fd < 0)
    {
        perror("Socket Error");
        return;
    }

    // setting socket to non-blocking mode so that server can handle multiple clients at the same time
    int flags = fcntl(socket_fd, F_GETFL, 0);
    fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK);

    // binding socket to ip add and port
    // -> sockaddr_in is a struct that contains the address family, ip address and port number
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = in_addr_t{0X00000000}; // ip address for telling the server to recieve request from all ips working on this machine
    address.sin_port = htons(8080);

    int socket_bind = bind(socket_fd, (sockaddr *)&address, sizeof(address));
    cout << "Socket bind result: " << socket_bind << endl;
    if (socket_bind < 0)
    {
        perror("Bind Error");
        return;
    }

    // listen for incoming connections
    // listen(socket to be listened on, max number of connections allowed in the queue)
    int socket_listen = listen(socket_fd, 5);
    cout << "Socket listen result: " << socket_listen << endl;
    if (socket_listen < 0)
    {
        perror("Listen Error");
        return;
    }

    cout << "Server running" << endl
         << "http://localhost:8080" << endl;

    // initializing the master set and adding the socket_fd to it so that server can listen to incoming connections on this socket
    FD_ZERO(&master_set);
    FD_SET(socket_fd, &master_set);

    int max_fd = socket_fd; // variable to keep track of the maximum file discriptor in the master set

    // creating seperate socket for each client connection
    while (true)
    {
        read_set = master_set; // copy the master set to the read set so that we can monitor the read set for incoming connections and data to be read

        int activity = select(max_fd + 1, &read_set, NULL, NULL, NULL);
        if (activity < 0)
        {
            perror("select error : ");
            continue;
        }
        for (int i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &read_set))
            { // checks whether the ith bit is set or not in the read set
                if (i == socket_fd)
                {
                    // this means there is an incoming connection because listening socket is ready to be read which means there is an incoming connection to be accepted
                    sockaddr_in client_address{};
                    socklen_t client_address_len = sizeof(client_address);
                    sockaddr *client_address_ptr = (sockaddr *)&client_address;

                    int client_socket_fd = accept(socket_fd, client_address_ptr, &client_address_len);
                    if (client_socket_fd < 0)
                    {
                        perror("Accept Error : ");
                        continue;
                    }
                    // making clients non blocking
                    int flags = fcntl(client_socket_fd, F_GETFL, 0);
                    fcntl(client_socket_fd, F_SETFL, flags | O_NONBLOCK);

                    cout << "Client connected with fd: " << client_socket_fd << endl;
                    FD_SET(client_socket_fd, &master_set);
                    if (client_socket_fd > max_fd)
                        max_fd = client_socket_fd; // updating the maximum file discriptor in the master set if the new client socket fd is greater than the current max fd
                    
                }
                else
                {
                    handle_client(i); // this means there is data to be read from the client with fd i
                }
            }
        }

    }
    return;
}

void Server::handle_client(int client_socket_fd)
{
        // reading till the full request is recieved as tcp transfer data in a stream
        Client &client = clients[client_socket_fd];
        char buffer[4096]; // as per the standard size of a page -> 4kb(4096 bytes) char size -> 1B
            // reading the request from the client
            ssize_t n = recv(client_socket_fd, buffer, sizeof(buffer), 0);
            if (n < 0)
            {
                if(errno == EWOULDBLOCK || errno == EAGAIN){
                    // this means there is no more data to be read from the client at the moment and we can break the loop and wait for the next activity on this client socket fd in the select loop
                    return;
                }
                perror("Recieving Error : ");
                close(client_socket_fd);
                FD_CLR(client_socket_fd, &this->master_set);
                clients.erase(client_socket_fd);
                return;
            }
            if (n == 0){
                close(client_socket_fd);
                FD_CLR(client_socket_fd, &this->master_set);
                clients.erase(client_socket_fd);
                return;
            }
            client.read_buffer.append(buffer, n);
            if (client.read_buffer.find("\r\n\r\n") != string::npos){
                client.header_complete = true;
            }
        if(client.header_complete == true){
        // parsing the request to get the requested path, method and http version
        string path, method, version;
        ParseHTTP parsedRequest = ParseHTTP(client.read_buffer);
        path = parsedRequest.getPath();
        method = parsedRequest.getMethod();
        version = parsedRequest.getVersion();

        if (method == "GET")
        {
            client.write_buffer = mapRouteGet(path, client_socket_fd);
            cout  << client.write_buffer << endl;
            send(client_socket_fd, client.write_buffer.c_str(), client.write_buffer.size(), 0); // sending the response to the client
        }
        else
        {
            string response = Response::getResponse("Connection closed!", 405, "text/plain");
            send(client_socket_fd, response.c_str(), response.size(), 0);
        }

        // closing the tcp connection with this->client so that server can listen to another client in the queue
        close(client_socket_fd);
        FD_CLR(client_socket_fd, &this->master_set);
        clients.erase(client_socket_fd);
    }
}
