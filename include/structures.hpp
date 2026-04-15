#pragma once
#include<unordered_map>
#include<string>
#include <iostream>

using namespace std;


struct Request{
    string method, path, version;
    unordered_map<string, string> header;
    string body;

    void printRequest(){
        cout << method << " " << path << " " << version << endl;
        for(auto& h : header){
            cout << h.first << ": " << h.second << endl;
        }
    }
};

struct Response{
    string body;
    int statusCode;
    string mime_type;
    string connection_header;
};