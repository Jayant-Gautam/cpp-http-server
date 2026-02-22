#include<string>
#include "mimeType.hpp"
using namespace std;

string get_mime_type(const string& path) {
    if (endsWith(path,".html")) return "text/html";
    if (endsWith(path,".css"))  return "text/css";
    if (endsWith(path,".js"))   return "application/javascript";
    if (endsWith(path,".png"))  return "image/png";
    if (endsWith(path,".jpg"))  return "image/jpeg";
    return "application/octet-stream";
}

// helper function to check if a string ends with a given suffix
bool endsWith(const string& str, const string& suffix) {
    if (str.length() < suffix.length()) return false;
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}