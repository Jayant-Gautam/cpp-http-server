#pragma once
#include<string>
#include<unordered_map>
#include "structures.hpp"

using namespace std;
class ParseHTTP {
    public:
        ParseHTTP(string& request);
        Request getRequest();
    private:
        Request req;
};