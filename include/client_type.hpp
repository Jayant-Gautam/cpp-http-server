#pragma once
using namespace std;

struct Client {
    std::string read_buffer;
    std::string write_buffer;
    bool header_complete = false;
    bool response_ready = false;
    size_t bytes_sent = 0;
    bool keep_alive = false;

    

};