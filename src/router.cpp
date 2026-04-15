#include "router.hpp"
using namespace std;

void Router::add_route(string method, string path, handler h)
{
    route_map[method][path] = h;
}

handler *Router::get_handler(string method, string path)
{
    if (route_map.find(method) != route_map.end())
    {
        if (route_map[method].find(path) != route_map[method].end())
        {
            return &route_map[method][path];
        }
    }
    return nullptr;
}