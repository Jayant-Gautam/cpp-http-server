#include "mapRoute.hpp"
#include <fstream>
#include "response.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include "mimeType.hpp"

using namespace std;

void mapRouteGet(string& path, int* socket_client_fd){
    path = "../www" + path;
    if(path == "../www/")
        path = "../www/index.html";
    ifstream file(path);
    string response;
    if(!file.is_open()){
        path = "../www/pageNotFound.html";
        file.open(path);
        string response_body = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        string mime_type = get_mime_type(path);
        response = Response::getResponse(response_body, 404, mime_type);
        file.close();
    }
    else{

        string response_body = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        string mime_type = get_mime_type(path);
        response = Response::getResponse(response_body, 200, mime_type);
        file.close();
    }
    cout << response << endl;
    send(*socket_client_fd, response.c_str(), response.size(), 0);
}