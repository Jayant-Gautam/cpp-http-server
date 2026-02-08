#include <iostream>
#include <string>
#include <unistd.h>
#include <netinet/in.h>
#include <vector>
#include "server.hpp"
#include "response.hpp"
#include "parseHTTP.hpp"
#include "mapRoute.hpp"


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

    // binding socket to ip add and port
    // -> sockaddr_in is a struct that contains the address family, ip address and port number
    sockaddr_in *address = new sockaddr_in();
    address->sin_family = AF_INET;
    address->sin_addr.s_addr = in_addr_t{0X00000000};
    address->sin_port = htons(8080);

    int socket_bind = bind(socket_fd, (sockaddr *)address, sizeof(*address));
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

    // creating seperate socket for each client connection
    while (true)
    {
        sockaddr_in client_address{};
        socklen_t client_address_len = sizeof(client_address);
        sockaddr* client_address_ptr = (sockaddr*)&client_address;

        int socket_client_fd = accept(socket_fd, client_address_ptr, &client_address_len);

        cout << "Client connected with fd: " << socket_client_fd << endl;
        if (socket_client_fd < 0)
        {
            perror("Accept Error");
            continue;
        }
        string request;
        char buffer[4096]; // as per the standard size of a page -> 4kb(4096 bytes) char size -> 1B

        // reading till the full request is recieved as tcp transfer data in a stream
        while(true){
            // reading the request fromt the client
            ssize_t n = recv(socket_client_fd, buffer, sizeof(buffer), 0);
            if(n < 0){
                perror("Recieving Error : ");
                break;
            }
            if(n == 0) break;
            request.append(buffer,n);
            if(request.find("\r\n\r\n") != string::npos) break;
        }


        // printing the request recieved from the client
        cout << "Request: " << endl;
        for (auto it : request)
        {
            cout << it;
        }
        cout << endl;

        // parsing the request to get the requested path, method and http version
        string path, method, version;
        ParseHTTP* parsedRequest = new ParseHTTP(request);
        path = parsedRequest->getPath();
        method = parsedRequest->getMethod();
        version = parsedRequest->getVersion();

        if(method == "GET"){
            mapRouteGet(path, &socket_client_fd);
        }


        // // sending response to the client
        // string response_body;
        // if(path == "/"){
        //     response_body = "<html><body><h1>Hello from C++ Server</h1></body></html>";
        // }
        // else if(path == "/about"){
        //     response_body = "<html><body><h1>About Page</h1><p>This is a simple http server implemented in C++ using sockets.</p></body></html>";
        // }
        // else if(path == "/showrequest"){
        //     response_body = "<html><body><h1>Request Details</h1><p>Method: " + method + "</p><p>Path: " + path + "</p><p>Version: " + version + "</p></body></html>";
        // }
        // else{
        //     response_body = "<html><body><h1>404 Not Found</h1><p>The requested resource was not found on this server.</p></body></html>";
        // }

        // string response = Response::getResponse(response_body, 200);
        // send(socket_client_fd, response.c_str(), response.size(), 0);

        // closing the tcp connection with this->client so that server can listen to another client in the queue
        close(socket_client_fd);
    }
    return;
}