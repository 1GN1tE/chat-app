#include "client.hpp"

Client::Client(int id, const std::string &user, const std::string &nick, const std::vector<int> &channels, const std::vector<int> &clients, int sd, int channel, bool admin, const struct sockaddr_in &addr)
{
    this->client_id = id;
    this->username = user;
    this->nickname = nick;
    this->client_sd = sd;
    this->curr_channel = 0;
    this->isAdmin = admin;
    this->remote_addr = addr;
}

Client::Client(int sd, const struct sockaddr_in &addr)
    : client_id(0),
      username(""),
      nickname(""),
      client_sd(sd),
      curr_channel(0),
      isAdmin(false),
      remote_addr(addr)
{
}

Client::~Client() {}

// Getters
int Client::getID() const { return client_id; }
std::string Client::getUserName() const { return username; }
std::string Client::getNickName() const { return nickname; }
int Client::getClientfd() const { return client_sd; }
bool Client::getAuth() const { return isAdmin; }
bool Client::isAuthenticated() const { return !username.empty(); }
int Client::getChannel() const { return curr_channel; }

// Setters
void Client::setID(int ID) { this->client_id = ID; }
void Client::setUserName(const std::string &UserName) { this->username = UserName; }
void Client::setNickName(const std::string &NickName) { this->nickname = NickName; }
void Client::setClientfd(int clientfd) { this->client_sd = clientfd; }
void Client::setAuth(bool Auth) { this->isAdmin = Auth; }

// TODO
std::string Client::getUserInfo() const
{
    return "Username: " + username + ", Nickname: " + nickname;
}