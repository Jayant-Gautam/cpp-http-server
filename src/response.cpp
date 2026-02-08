#include "response.hpp"

string Response::getResponse(string response_body, int statusCode, string mime_type) {
    string statusText =
        (statusCode == 200 ? "OK" :
        (statusCode == 404 ? "Not Found" : "Internal Server Error"));

    string response =
        "HTTP/1.1 " + to_string(statusCode) + " " + statusText + "\r\n"
        "Content-Length: " + to_string(response_body.size()) + "\r\n"
        "Content-Type: " + mime_type + "\r\n"
        "Connection: close\r\n"
        "\r\n" +
        response_body;

    return response;
}
