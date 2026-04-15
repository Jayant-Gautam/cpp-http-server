#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include "structures.hpp"

using namespace std;

using handler = function<void(Request&, Response &res)>;

class Router
{
private:
    unordered_map<string, unordered_map<string, handler>> route_map;
public:
    void add_route(string method, string path, handler h);
    handler* get_handler(string method, string path);

};