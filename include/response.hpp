#pragma once
#include<string>
#include "client_type.hpp"

using namespace std;
class Res {
    int statusCode = 200;
public:
    static string getResponse(string response_body, int statusCode = 200, string mime_type = "text/html", string connection_header = "close");
};