#pragma once
#include<string>
#include<unordered_map>
using namespace std;
class ParseHTTP {
    public:
        ParseHTTP(string& request);
        string getMethod();
        string getPath();
        string getVersion();
        string getHeader(string header_key);
    private:
        string method, path, version;
        unordered_map<string, string> header; 
};