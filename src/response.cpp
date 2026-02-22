#include "response.hpp"

string Response::getResponse(string response_body, int statusCode, string mime_type) {
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

    string response =
        "HTTP/1.1 " + to_string(statusCode) + " " + statusText + "\r\n"
        "Content-Length: " + to_string(response_body.size()) + "\r\n"
        "Content-Type: " + mime_type + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        response_body;

    return response;
}
