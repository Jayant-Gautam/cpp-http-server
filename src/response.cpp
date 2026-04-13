#include "response.hpp"
#include "server.hpp"

string Response::getResponse(string response_body, int statusCode, string mime_type, Client &client_socket) {
    string statusText = "";
    switch (statusCode) {
        case 200 : 
            statusText = "OK";
            break;
        case 404 : 
            statusText = "Not Found";
            break;
        case 405 : 
            statusText = "Method Not Allowed";
            break;
        // can add more status codes later
        default : 
            statusText = "Internal Server Error";
    }
    string connection_header = "null";
    if(client_socket.keep_alive){
        connection_header = "Connection: keep-alive\r\n";
    } 
    else{
        connection_header = "Connection: close\r\n";
    }


    string response =
        "HTTP/1.1 " + to_string(statusCode) + " " + statusText + "\r\n"
        "Content-Length: " + to_string(response_body.size()) + "\r\n"
        "Content-Type: " + mime_type + "\r\n"
        + connection_header +
        "\r\n" +
        response_body;

    return response;
}
