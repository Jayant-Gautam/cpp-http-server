#pragma once
#include <queue>
#include "structures.hpp"
using namespace std;

enum parseState{
    READING_HEADERS,
    READING_BODY
};

struct Client {
    parseState state = READING_HEADERS;
    std::string read_buffer;
    queue<std::string> write_buffer;
    std::string body;
    size_t content_length = 0;
    size_t body_received = 0;
    Request current_request;

    size_t bytes_sent = 0;
    bool keep_alive = false;
};