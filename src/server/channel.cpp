#include "channel.hpp"

// User created new channel
Channel::Channel(int channel_id, const std::string &name, const std::string &description, const std::string &key, int creatorID)
{
    this->channel_id = 0;            // Default channel_id
    this->name = name;               // Set name
    this->description = description; // Set description
    this->key = key;                 // Set key
    this->creatorID = creatorID;     // Set creatorID
    this->members = {creatorID};     // Initialize members with creatorID
    this->admins = {creatorID};      // Initialize admins with creatorID
}

// Load channel from database
Channel::Channel(int channel_id, const std::string &name, const std::string &description, const std::string &key, int creatorID, const std::vector<int> &members, const std::vector<int> &admins)
{
    this->channel_id = channel_id;
    this->name = name;
    this->description = description;
    this->key = key;
    this->creatorID = creatorID;
    this->members = members;
    this->admins = admins;
}

// Getters
int Channel::getId() const { return channel_id; }
std::string Channel::getName() const { return name; }
std::string Channel::getDescription() const { return description; }
std::string Channel::getKey() const { return key; }
int Channel::getCreatorID() const { return creatorID; }

// Check if a client_id is a member
bool Channel::isMember(int client_id) const
{
    return std::find(members.begin(), members.end(), client_id) != members.end();
}

// Check if a client_id is an admin
bool Channel::isAdmin(int client_id) const
{
    return std::find(admins.begin(), admins.end(), client_id) != admins.end();
}

// Setters
void Channel::setName(const std::string &name) { this->name = name; }
void Channel::setDescription(const std::string &description) { this->description = description; }
void Channel::setKey(const std::string &key) { this->key = key; }
void Channel::setCreatorID(int id) { this->creatorID = id; }

// Add a client as a member
bool Channel::addMember(int client_id, Database &db)
{
    // Only add if client is not already a member
    if (std::find(members.begin(), members.end(), client_id) != members.end())
    {
        return false;
    }

    if (!db.addMemberToChannel(this->channel_id, client_id, "member"))
    {
        return false;
    }

    members.push_back(client_id);
    return true;
}

// Add a client as an admin
bool Channel::addAdmin(int client_id, Database &db)
{
    // Only add if client is not already an admin
    if (std::find(admins.begin(), admins.end(), client_id) != admins.end())
    {
        return false;
    }

    if (!db.addMemberToChannel(this->channel_id, client_id, "admin"))
    {
        return false;
    }

    admins.push_back(client_id);
    return true;
}
