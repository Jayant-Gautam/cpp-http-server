#include "parseHTTP.hpp"
#include "structures.hpp"
#include <sstream>

ParseHTTP::ParseHTTP(string& request){
    int first_line_end = request.find("\r\n");
    string request_first_line = request.substr(0, first_line_end);
    istringstream request_stream(request_first_line); // istringstream is a stream class to operate on strings. It allows us to read data from a string as if it were a stream (like cin or file stream).
    request_stream >> this->req.method >> this->req.path >> this->req.version;

    // parsing the headers and storing them in a map for easy access
    size_t header_start = first_line_end + 2; // +2 to skip the \r\n
    size_t header_end = request.find("\r\n\r\n"); // headers end with a blank line (\r\n\r\n)
    string headers_str = request.substr(header_start, header_end - header_start);
    istringstream headers_stream(headers_str);
    string header_line;
    while (getline(headers_stream, header_line)) {
        size_t delimiter_pos = header_line.find(": ");
        if (delimiter_pos != string::npos) {
            string header_key = header_line.substr(0, delimiter_pos);
            string header_value = header_line.substr(delimiter_pos + 2); // +2 to skip the ": "
            this->req.header[header_key] = header_value;
        }
    }
}



Request ParseHTTP::getRequest(){
    return this->req;
}