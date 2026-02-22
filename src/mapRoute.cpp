#include "mapRoute.hpp"
#include <fstream>
#include "response.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include "mimeType.hpp"

using namespace std;

void mapRouteGet(string& path, int socket_client_fd){
    // mapping the path to the file in the www directory
    path = "../www" + path;
    if(path == "../www/")
        path = "../www/index.html";
    ifstream file(path);
    string response;

    // if the file is not found, send 404 response
    if(!file.is_open()){
        path = "../www/pageNotFound.html";
        file.open(path);
        string response_body = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        string mime_type = get_mime_type(path);
        response = Response::getResponse(response_body, 404, mime_type);
        file.close();
    }
    // if the file is found, send the file content as response
    else{
        string response_body = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        string mime_type = get_mime_type(path);
        response = Response::getResponse(response_body, 200, mime_type);
        file.close();
    }

    // cout << response << endl;
    send(socket_client_fd, response.c_str(), response.size(), 0); // sending the response to the client
}