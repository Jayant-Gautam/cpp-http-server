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
#include "client_type.hpp"

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
        FD_ZERO(&write_set);
        for (auto &[fd, client] : clients)
        {
            if (!client.write_buffer.empty())
            {
                FD_SET(fd, &write_set);
            }
        }

        int activity = select(max_fd + 1, &read_set, &write_set, NULL, NULL);
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
                    handle_client_read(i); // this means there is data to be read from the client with fd i
                }
            }
            if (FD_ISSET(i, &write_set))
            {
                handle_client_write(i);
            }
        }
    }
    return;
}

void Server::handle_client_read(int client_socket_fd)
{
    // reading till the full request is recieved as tcp transfer data in a stream

    Client &client = clients[client_socket_fd];

    char buffer[4096]; // as per the standard size of a page -> 4kb(4096 bytes) char size -> 1B

    size_t total_bytes_read = 0;
    size_t allowed_limit = 64 * 1024; // setting a limit of 64kb for the request size to prevent denial of service attack by sending very large requests

    while (total_bytes_read < allowed_limit)
    {
        // reading the request from the client
        ssize_t n = recv(client_socket_fd, buffer, sizeof(buffer), 0);
        if (n < 0)
        {
            if (errno == EWOULDBLOCK || errno == EAGAIN)
            {
                // this means there is no more data to be read from the client at the moment and we can break the loop and wait for the next activity on this client socket fd in the select loop
                break;
            }
            perror("Recieving Error : ");
            close(client_socket_fd);
            FD_CLR(client_socket_fd, &this->master_set);
            clients.erase(client_socket_fd);
            return;
        }
        if (n == 0)
        {
            close(client_socket_fd);
            FD_CLR(client_socket_fd, &this->master_set);
            clients.erase(client_socket_fd);
            return;
        }
        total_bytes_read += n;
        client.read_buffer.append(buffer, n);
    }

    while (true)
    {
        auto pos = client.read_buffer.find("\r\n\r\n");
        if (pos == string::npos)
            break;

        string request = client.read_buffer.substr(0, pos + 4);
        client.read_buffer.erase(0, pos + 4); // removing the header from the read buffer so that if there is any body in the request, it will be in the read buffer and we can process it later if needed

        // printing the request
        cout << "******REQUEST START******" << endl;
        cout << request << endl;
        cout << "******REQUEST END******" << endl;

        // parsing the request to get the requested path, method and http version
        string path, method, version;
        ParseHTTP parsedRequest = ParseHTTP(request);
        path = parsedRequest.getPath();
        method = parsedRequest.getMethod();
        version = parsedRequest.getVersion();
        string connection = parsedRequest.getHeader("Connection");
        if (version == "HTTP/1.1")
        {
            if (connection == "close")
            {
                client.keep_alive = false;
            }
            else
            {
                client.keep_alive = true;
            }
        }
        else
        {
            if (connection == "keep-alive")
            {
                client.keep_alive = true;
            }
            else
            {
                client.keep_alive = false;
            }
        }

        if (method == "GET")
        {
            client.write_buffer.push(mapRouteGet(path, clients[client_socket_fd]));
            // cout << client.write_buffer << endl;
        }
        else
        {
            string response = Response::getResponse("Connection closed!", 405, "text/plain");
            client.write_buffer.push(response);
        }
    }
}

void Server::handle_client_write(int client_socket_fd)
{
    Client &client = clients[client_socket_fd];
    if (client.write_buffer.empty())
        return; // this means there is no response to be sent to the client at the moment and we can return and wait for the next activity on this client socket fd in the select loop
    string &current_response = client.write_buffer.front();
    ssize_t n = send(client_socket_fd, current_response.c_str() + client.bytes_sent, current_response.size() - client.bytes_sent, 0);
    if (n < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // this means the socket is not ready to be written to at the moment and we can break the loop and wait for the next activity on this client socket fd in the select loop
            return;
        }
        perror("Sending Error : ");
        close(client_socket_fd);
        FD_CLR(client_socket_fd, &this->master_set);
        clients.erase(client_socket_fd);
        return;
    }
    client.bytes_sent += n;
    if (client.bytes_sent == current_response.size())
    {
        reset_client(client);
        // this means the full response has been sent to the client and we can close the connection if the client does not want to keep the connection alive or if there is no more response to be sent to the client
        if (client.keep_alive == false && client.write_buffer.empty())
        {
            close(client_socket_fd);
            FD_CLR(client_socket_fd, &this->master_set);
            clients.erase(client_socket_fd);
        }
    }
}

void Server::reset_client(Client &client)
{
    client.write_buffer.pop();
    client.bytes_sent = 0;
}
