#pragma once
#include<string>
using namespace std;
class ParseHTTP {
    public:
        ParseHTTP(string& request);
        string getMethod();
        string getPath();
        string getVersion();
    private:
        string method, path, version;
};