#include "server.hpp"
using namespace std;

const int port = 8080;

int main() {
    Server server(port);
    server.run();
    return 0;
}
