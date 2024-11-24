#pragma once

#include <string>
#include <vector>
#include <netinet/in.h>
#include <algorithm>
#include <mutex>

class Client
{
private:
    // Values from database
    int client_id;
    std::string username;
    std::string nickname;

    // Values even server running
    int client_sd;
    int curr_channel;
    bool isAdmin;
    struct sockaddr_in remote_addr;

public:
    std::mutex mutex;

public:
    Client(int sd, const struct sockaddr_in &addr);
    Client(int id, const std::string &user, const std::string &nick, const std::vector<int> &channels, const std::vector<int> &clients, int sd, int channel, bool admin, const struct sockaddr_in &addr);
    ~Client();

    // Getters
    int getID() const;
    std::string getUserName() const;
    std::string getNickName() const;
    int getClientfd() const;
    bool getAuth() const;
    std::string getUserInfo() const;
    bool isAuthenticated() const;
    int getChannel() const;

    // Setters
    void setID(int ID);
    void setUserName(const std::string &UserName);
    void setNickName(const std::string &NickName);
    void setClientfd(int clientfd);
    void setAuth(bool Auth);
};