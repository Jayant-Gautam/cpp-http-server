#pragma once
#include<string>
using namespace std;
class Response {
    int statusCode = 200;
public:
    static string getResponse(string, int statusCode = 200);
};