#pragma once

#include <string>
#include <vector>
#include <algorithm>
#include "database.hpp"

class Channel
{
private:
    int channel_id;
    std::string name;
    std::string description;
    std::string key;
    int creatorID;
    std::vector<int> members;
    std::vector<int> admins;

public:
    Channel(int channel_id, const std::string &name, const std::string &description, const std::string &key, int creatorID);
    Channel(int channel_id, const std::string &name, const std::string &description, const std::string &key, int creatorID, const std::vector<int> &members, const std::vector<int> &admins);

    // Getters
    int getId() const;
    std::string getName() const;
    std::string getDescription() const;
    std::string getKey() const;
    int getCreatorID() const;
    bool isMember(int client_id) const;
    bool isAdmin(int client_id) const;

    // Setters
    void setName(const std::string &name);
    void setDescription(const std::string &description);
    void setKey(const std::string &key);
    void setCreatorID(int id);

    // Modifiers
    bool addMember(int client_id, Database &db);
    bool addAdmin(int client_id, Database &db);
};
