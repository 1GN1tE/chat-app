#pragma once

#include <sqlite3.h>
#include <iostream>
#include <vector>
#include <string>
#include "db_pool.hpp"

#define PAGE_SZ 10

class Database
{
private:
    ConnectionPool pool;

public:
    Database(const std::string &dbName, int poolSize);
    ~Database();

    // Client functions
    bool insertClient(const std::string &username, const std::string &password, int &client_id);
    bool getClientByUsername(const std::string &username, int &clientId, std::string &password, std::string &nickname);
    bool getClientByUsername(const std::string &username, int &clientId);

    // Channel functions
    bool insertChannel(const std::string &channelName, const std::string &description, int ownerId, const std::string &key);
    bool getAllChannelIds(std::vector<int> &channelIds);
    bool getChannel(int channel_id, std::string &name, std::string &description, std::string &key, int &creatorID, std::vector<int> &members, std::vector<int> &admins);
    bool addMemberToChannel(int channel_id, int client_id, const std::string &role);
    
    // Message functions
    bool insertChannelMessage(int sender_id, int channel_id, const std::string &message_text);
    bool insertPrivateMessage(int sender_id, int recipient_id, const std::string &message_text);
    bool getChannelMessagesPaginated(int channel_id, int page, std::vector<std::tuple<std::string, std::string, std::string>>& messages);
    bool getPrivateMessagesPaginated(int id_a, int id_b, int page, std::vector<std::tuple<std::string, std::string, std::string>>& messages);
    bool getPrvMsgIds(int client_id, std::vector<int> &ids);

    // File functions
    bool insertFile(const std::string &filename, int senderId, int recipientId, int channelId, const std::string &uuid);
    bool getFileByUUID(const std::string &uuid, std::string &filename, int &recipientId, int &channelId);

};
