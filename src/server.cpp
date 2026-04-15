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
#include <sys/epoll.h>
#include "client_type.hpp"
#include "structures.hpp"

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
    address.sin_port = htons(this->port);

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

    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("epoll_create1");
        return;
    }

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = socket_fd;

    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socket_fd, &ev);

    const int max_events = 1024;
    epoll_event events[max_events];

    // creating seperate socket for each client connection
    while (true)
    {
        int n = epoll_wait(epoll_fd, events, max_events, -1);
        if (n == -1)
        {
            perror("epoll_wait");
            return;
        }
        for (int i = 0; i < n; i++)
        {
            int fd = events[i].data.fd;

            if (events[i].events & (EPOLLERR | EPOLLHUP))
            {
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                clients.erase(fd);
                continue;
            }
            if (fd == socket_fd)
            {
                while (true)
                {
                    // this means there is an incoming connection because listening socket is ready to be read which means there is an incoming connection to be accepted
                    sockaddr_in client_address{};
                    socklen_t client_address_len = sizeof(client_address);
                    sockaddr *client_address_ptr = (sockaddr *)&client_address;

                    int client_socket_fd = accept(socket_fd, client_address_ptr, &client_address_len);
                    if (client_socket_fd < 0)
                    {
                        if (errno == EAGAIN || errno == EWOULDBLOCK)
                            break;
                        perror("Accept Error");
                        break;
                    }
                    // making clients non blocking
                    int flags = fcntl(client_socket_fd, F_GETFL, 0);
                    fcntl(client_socket_fd, F_SETFL, flags | O_NONBLOCK);

                    cout << "Client connected with fd: " << client_socket_fd << endl;

                    // register in epoll
                    epoll_event ev{};
                    ev.events = EPOLLIN | EPOLLET; // Edge Triggered mode for better performance, we will read all the data from the client socket until there is no more data to be read in one go instead of waiting for the next activity on the client socket fd in the select loop
                    ev.data.fd = client_socket_fd;

                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket_fd, &ev);

                    clients[client_socket_fd] = Client();
                }
            }
            else
            {
                if (events[i].events & EPOLLIN)
                {
                    handle_client_read(fd, epoll_fd);
                }
                if (events[i].events & EPOLLOUT)
                {
                    handle_client_write(fd, epoll_fd);
                }
            }
        }
    }
    return;
}

void Server::handle_client_read(int client_socket_fd, int epoll_fd)
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
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket_fd, nullptr);
            close(client_socket_fd);
            clients.erase(client_socket_fd);
            return;
        }
        if (n == 0)
        {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket_fd, nullptr);
            close(client_socket_fd);
            clients.erase(client_socket_fd);
            return;
        }
        total_bytes_read += n;
        client.read_buffer.append(buffer, n);
    }

    while (true)
    {
        if (client.state == READING_HEADERS)
        {
            auto pos = client.read_buffer.find("\r\n\r\n");
            if (pos == string::npos)
                break;

            string header_part = client.read_buffer.substr(0, pos + 4);
            client.read_buffer.erase(0, pos + 4);

            ParseHTTP parsedRequest(header_part);
            Request req = parsedRequest.getRequest();

            //check for connection header to decide whether to keep the connection alive or not after sending the response
            string connection = req.header["Connection"];

            if (req.version == "HTTP/1.1")
                client.keep_alive = (connection != "close");
            else
                client.keep_alive = (connection == "keep-alive");

            // if the request is a POST request, we need to read the body of the request before routing it to the handler function, otherwise if it is a GET request, we can route it to the handler function immediately without reading the body as GET requests do not have a body
            if (req.method == "POST")
            {
                string cl = req.header["Content-Length"];
                client.content_length = cl.empty() ? 0 : std::stoul(cl);
                client.body_received = 0;
                client.body.clear();

                client.state = READING_BODY;

                // store req temporarily
                client.current_request = req;
            }
            else
            {
                // GET request does not have body so we can route it to the handler function immediately
                string response = routeHandler(req, router);
                client.write_buffer.push(response);

                epoll_event ev{};
                ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                ev.data.fd = client_socket_fd;

                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_socket_fd, &ev);
            }
        }

        else if (client.state == READING_BODY)
        {
            size_t remaining = client.content_length - client.body_received;

            if (client.read_buffer.size() == 0)
                break;

            size_t to_copy = min(remaining, client.read_buffer.size());

            client.body.append(client.read_buffer.substr(0, to_copy));
            client.read_buffer.erase(0, to_copy);

            client.body_received += to_copy;

            if (client.body_received == client.content_length)
            {
                // the full body has been received, we can now route the request to the handler function
                client.current_request.body = client.body;

                string response = routeHandler(client.current_request, router);
                client.write_buffer.push(response);

                epoll_event ev{};
                ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                ev.data.fd = client_socket_fd;

                epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_socket_fd, &ev);

                // reset for next request
                client.state = READING_HEADERS;
                client.body.clear();
                client.content_length = 0;
                client.body_received = 0;
            }
        }
    }
}

void Server::handle_client_write(int client_socket_fd, int epoll_fd)
{
    Client &client = clients[client_socket_fd];

    if (client.write_buffer.empty())
        return; // this means there is no response to be sent to the client at the moment and we can return and wait for the next activity on this client socket fd in the select loop

    string &current_response = client.write_buffer.front();
    cout << "Sending response to client with fd " << client_socket_fd << " : " << endl
         << current_response << endl;
    ssize_t n = send(client_socket_fd, current_response.c_str() + client.bytes_sent, current_response.size() - client.bytes_sent, 0);
    cout << "Bytes sent: " << n << endl;
    if (n < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            // this means the socket is not ready to be written to at the moment and we can break the loop and wait for the next activity on this client socket fd in the select loop
            return;
        }
        perror("Sending Error : ");
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket_fd, nullptr);
        close(client_socket_fd);
        clients.erase(client_socket_fd);
        return;
    }

    client.bytes_sent += n;

    if (client.bytes_sent == current_response.size())
    {
        reset_client(client);
        // this means the full response has been sent to the client and we can close the connection if the client does not want to keep the connection alive or if there is no more response to be sent to the client
        if (client.write_buffer.empty())
        {
            epoll_event ev{};
            ev.events = EPOLLIN | EPOLLET;
            ev.data.fd = client_socket_fd;

            epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client_socket_fd, &ev);
        }

        if (client.keep_alive == false && client.write_buffer.empty())
        {
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket_fd, nullptr);
            close(client_socket_fd);
            clients.erase(client_socket_fd);
        }
    }
}

void Server::reset_client(Client &client)
{
    client.write_buffer.pop();
    client.bytes_sent = 0;
}

Server::Server(int port) : port(port)
{
    this->port = port;

    router.add_route("GET", "/func", [](Request &req, Response &res) -> void
                     {
        res.statusCode = 200;
        res.mime_type = "text/plain";
        res.connection_header = "close";
        res.body = "Hello, World!"; });
    
    router.add_route("POST", "/echo", [](Request &req, Response &res) -> void
                     {
        res.statusCode = 200;
        res.mime_type = "application/json";
        res.connection_header = "close";
        res.body = "okay response"; });
}
