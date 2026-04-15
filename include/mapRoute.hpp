#pragma once
using namespace std;
#include<string>
#include "client_type.hpp"
#include "response.hpp"
#include <nlohmann/json.hpp>
#include "structures.hpp"
#include "router.hpp"

using json = nlohmann::json;

Response routeFile(Request& req);
bool checkIfFileExists(const string& reqPath);
Response dispatch(Request& req, Router& router);
string routeHandler(Request& req, Router& router);
