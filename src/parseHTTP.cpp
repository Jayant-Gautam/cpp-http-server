#include "parseHTTP.hpp"
#include <sstream>

ParseHTTP::ParseHTTP(string& request){
    int first_line_end = request.find("\r\n");
    string request_first_line = request.substr(0, first_line_end);
    istringstream request_stream(request_first_line); // istringstream is a stream class to operate on strings. It allows us to read data from a string as if it were a stream (like cin or file stream).
    request_stream >> this->method >> this->path >> this->version;
}

string ParseHTTP::getMethod(){
    return this->method;
}
string ParseHTTP::getPath(){
    return this->path;
}
string ParseHTTP::getVersion(){
    return this->version;
}