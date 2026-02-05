#include "response.hpp"

string Response::getResponse(string response_body, int statusCode){
    string response =
    "HTTP/1.1 200 OK\r\n"
    "Content-Length: " + std::to_string(response_body.size()) + "\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n" +
    response_body;

    return response;
}