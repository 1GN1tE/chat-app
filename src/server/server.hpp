#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <format>
#include <map>
#include <mutex>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <fstream>
#include <chrono>
#include <random>
#include <sstream>

#include "helper.hpp"
#include "threadpool.hpp"
#include "client.hpp"
#include "channel.hpp"
#include "database.hpp"
#include "../protocol/message.hpp"

class Server
{
private:
    int socketfd;
    std::string name;
    ThreadPool pool;

    std::map<int, std::unique_ptr<Channel>> channels;
    std::map<int, std::unique_ptr<Client>> clients;

    std::mutex channelsMutex;
    std::mutex clientsMutex;

    int epoll_fd;
    std::vector<struct epoll_event> events;

    Database db;

private:
    // Initialisation Function
    void createSocket(int port);
    void initChannels(void);

public:
    Server(const std::string &name, int port, int threadPoolSize, int dbPoolSize);
    ~Server();

    // Running
    void startServer(void);
    void addClient();
    void removeClient(int client_sd);
    void clientRequest(int fd);
    void sendClient(int client_sd, const Message &msg);

    // Handlers
    void handleRequest(int client_sd, Message &msg);

    // User operations
    void login(int client_sd, Message &msg);
    // void register_(int client_sd, Message &msg);
    
    // Message handler
    void channelMsg(int client_sd, Message &msg);
    void getChannelMsg(int cliend_sd, Message &msg);
    void userMsg(int client_sd, Message &msg);
    void getUserMsg(int cliend_sd, Message &msg);

    void joinChannel(int cliend_sd, Message &msg);

    // Non Database operations
    void listChannels(int client_sd, Message &msg);
    void listOnlineUsers(int client_sd, Message &msg);
    void invalidCommand(int client_sd);

    // File transfer
    std::string generateUniqueId();
    void upload(int client_sd, Message &msg);
    void download(int client_sd, Message &msg);
    std::string savefile(std::string filename, std::string contents);
    bool getfile(const std::string& uniqueId, std::string& contents);

    // Others
    Channel *getChannel(const std::string &channel_name);
    Channel *getChannelById(int channel_id);
    Client *getClient(int client_sd);

};
