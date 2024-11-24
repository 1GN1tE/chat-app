#include "server.hpp"

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return -1;
    }
    Server srv("CS744", atoi(argv[1]), 1, 1);
    srv.startServer();
    return 0;
}
