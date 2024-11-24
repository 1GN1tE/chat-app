#include "helper.hpp"

const std::string date_time()
{
    time_t t;
    struct tm *tmp;
    char buf[100];
    time(&t);
    tmp = localtime(&t);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", tmp);
    return buf;
}

[[noreturn]] void throwError(const std::string& errorMessage) {
    std::cerr << errorMessage << std::endl;
    throw std::runtime_error(errorMessage);
}
