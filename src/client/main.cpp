#include "client.hpp"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <IP Address> <Port>" << std::endl;
        return 1;
    }

    std::string IP = argv[1];
    int port = std::stoi(argv[2]);

    Client client(IP, port);
    client.run();

    return 0;
}
