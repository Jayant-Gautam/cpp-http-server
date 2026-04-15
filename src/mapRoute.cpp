#include "mapRoute.hpp"
#include <fstream>
#include "response.hpp"
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>
#include "mimeType.hpp"
#include "server.hpp"
#include "structures.hpp"
#include "router.hpp"


using namespace std;

Response routeFile(Request& req){
    string path = req.path;
    string connectionHeader = req.header["Connection"];

    // mapping the path to the file in the www directory
    path = "../www" + path;
    if(path == "../www/")
        path = "../www/index.html";
    ifstream file(path);
    // string response;
    Response res;
    res.connection_header = connectionHeader;

    // if the file is not found, send 404 response
    if(!file.is_open()){
        path = "../www/pageNotFound.html";
        file.open(path);
        string response_body = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        string mime_type = get_mime_type(path);

        res.body = response_body;
        res.statusCode = 404;
        res.mime_type = mime_type;

        // response = Response::getResponse(response_body, 404, mime_type, client_socket);
        file.close();
    }
    // if the file is found, send the file content as response
    else{
        string response_body;
        try{
            response_body = string((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
        }
        catch(const exception& e){
            cerr << "Error: " << e.what() << endl;
            res.statusCode = 500;
            res.body = "Internal Server Error";
            res.mime_type = "text/plain";
            return res;
        }
        string mime_type = get_mime_type(path);

        res.body = response_body;
        res.statusCode = 200;
        res.mime_type = mime_type;

        // response = Response::getResponse(response_body, 200, mime_type, client_socket);
        file.close();
    }
    return res;
}

bool checkIfFileExists(const string& reqPath){
    string path = "../www" + reqPath;
    if(path == "../www/")
        path = "../www/index.html";
    try{
        ifstream file(path);
        bool exists = file.is_open();
        file.close();
        cout << exists << endl;
        return exists;
    }
    catch(const exception& e){
        cerr << "Error checking file existence: " << e.what() << endl;
        return false;
    }
}

Response dispatch(Request& req, Router& router){
    string path = req.path;
    string method = req.method;
    Response res;
    handler* h = router.get_handler(method, path); // this will return a pointer to the handler function if it exists, otherwise it will return nullptr
    res.mime_type = "text/plain";
    res.connection_header = "keep-alive";
    if(h){
        (*h)(req, res);
    }
    else{
        res.statusCode = 404;
        res.body = "Not Found";
    }

    return res;
}

string routeHandler(Request& req, Router& router){
    string path = req.path;
    if(checkIfFileExists(path)){
        Response res = routeFile(req);
        return Res::getResponse(res.body, res.statusCode, res.mime_type, res.connection_header);
    }
    else{
        Response res = dispatch(req, router);
        // res.body = res.body.dump(); // converting json to string
        return Res::getResponse(res.body, res.statusCode, res.mime_type, res.connection_header);
    }
}


