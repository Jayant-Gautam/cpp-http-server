#pragma once
#include<string>
#include "client_type.hpp"

using namespace std;
class Response {
    int statusCode = 200;
public:
    static string getResponse(string response_body, int statusCode = 200, string mime_type = "text/html", Client &client_socket = *new Client());
};