#pragma once

#include <iostream>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <vector>
#include <string>
#include <atomic>
#include <thread>
#include <sstream>
#include <fstream>
#include <filesystem>
#include <algorithm>

#include "../protocol/message.hpp"

class Client
{
private:
    std::string IP;
    int port;
    int clientSd;
    std::atomic<bool> stopReceiving;
    std::thread receiveThread;

    std::string channel;
    std::string user;

    std::string nick;
    std::string tmp;

private:
    void receiveMessages();
    bool connectToServer();
    void sendMessageLoop();

    void sendMessage(Message &msg);
    Message *handleCommand(const std::string &data);

    void handleResponse(Message &response);

    void channelMessage(const Message &msg);
    void userMessage(const Message &msg);
    void saveToFile(const Message &msg);

public:
    Client(const std::string &ip, int port);
    ~Client();
    void run();
};
