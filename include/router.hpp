#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include <nlohmann/json.hpp>
#include "structures.hpp"

using json = nlohmann::json;
using handler = function<void(Request&, Response &res)>;

class Router
{
private:
    /* data */
    unordered_map<string, unordered_map<string, handler>> route_map;
public:
    // Router(/* args */);
    // ~Router();
    void add_route(string method, string path, handler h){
        route_map[method][path] = h;
    }
    handler* get_handler(string method, string path){
        if(route_map.find(method) != route_map.end()){
            if(route_map[method].find(path) != route_map[method].end()){
                return &route_map[method][path];
            }
        }
        return nullptr;
    }

};