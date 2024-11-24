#include "server.hpp"

#define ERROR "\033[31m[x]\033[0m "
#define INFO "\033[34m[!]\033[0m "

void Server::handleRequest(int client_sd, Message &msg)
{
    // Handle the request
    switch (msg.getCommand())
    {
    case 0x10:
        std::cout << INFO << "Login from " << client_sd << std::endl;
        Server::login(client_sd, msg);
        break;

    case 0x11:
        std::cout << ERROR << "Set new password NOT IMPLEMENTED" << std::endl;
        break;

    case 0x12:
        std::cout << ERROR << "Change nickname NOT IMPLEMENTED" << std::endl;
        break;

    case 0x20:
        std::cout << INFO << "List Channels " << client_sd << std::endl;
        Server::listChannels(client_sd, msg);
        break;

    case 0x21:
        std::cout << INFO << "List Users " << client_sd << std::endl;
        Server::listOnlineUsers(client_sd, msg);
        break;

    case 0x22:
        std::cout << INFO << "Get user messages " << client_sd << std::endl;
        Server::getUserMsg(client_sd, msg);
        break;

    case 0x23:
        std::cout << INFO << "Get channel messages " << client_sd << std::endl;
        Server::getChannelMsg(client_sd, msg);
        break;

    case 0x30:
        std::cout << INFO << "Channel message from " << client_sd << std::endl;
        Server::channelMsg(client_sd, msg);
        break;

    case 0x31:
        std::cout << INFO << "User message from " << client_sd << std::endl;
        Server::userMsg(client_sd, msg);
        break;

    case 0x40:
        std::cout << INFO << "Join Channel " << client_sd << std::endl;
        Server::joinChannel(client_sd, msg);
        break;

    case 0x41:
        std::cout << ERROR << "Change channel NOT IMPLEMENTED" << std::endl;
        break;

    case 0x42:
        std::cout << ERROR << "Create channel NOT IMPLEMENTED" << std::endl;
        break;

    case 0x43:
        std::cout << ERROR << "Change channel topic NOT IMPLEMENTED" << std::endl;
        break;

    case 0x44:
        std::cout << ERROR << "Change channel key NOT IMPLEMENTED" << std::endl;
        break;

    case 0x50:
        std::cout << ERROR << "Kick user NOT IMPLEMENTED" << std::endl;
        break;

    case 0x51:
        std::cout << ERROR << "Ban user NOT IMPLEMENTED" << std::endl;
        break;

    case 0x60:
        std::cout << INFO << "File upload " << client_sd << std::endl;
        Server::upload(client_sd, msg);
        break;

    case 0x61:
        std::cout << INFO << "File Download " << client_sd << std::endl;
        Server::download(client_sd, msg);
        break;

    default:
        std::cout << ERROR << "Unknown request code: " << msg.getCommand() << std::endl;
        Server::invalidCommand(client_sd);
    }
}

// !login <username> <password>
void Server::login(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();
    std::string username, password, nickname;

    Message response;
    response.setType(0x02);

    // Check if already logged in
    if (client->isAuthenticated())
    {
        response.setCommand(0x10); // Login successful
        response.addArg(this->name);
        Server::sendClient(client_sd, response);
        return;
    }

    // Invalid Arguments
    if (args.size() < 2)
    {
        response.setCommand(0x00);
        Server::sendClient(client_sd, response);
        return;
    }

    username = args[0];

    // TODO check if another user is already logged in

    // Fetch details for the user with <username>
    int clientId;
    if (!db.getClientByUsername(username, clientId, password, nickname))
    {
        // If username not found then insert user
        if (db.insertClient(username, args[1], clientId))
        {
            std::cout << "Client addedd with username: " << username << std::endl;
            client->setUserName(username);
            client->setID(clientId);

            response.setCommand(0x011); // User created
            Server::sendClient(client_sd, response);
            return;
        }
        else
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            return;
        }
    }

    // std::cout << "Username: " << username << "\t" << "Password: " << password << "\t" << "Nickname: " << nickname << "\n";

    if (args[1] != password)
    {
        response.setCommand(0x012); // Incorrect password
        Server::sendClient(client_sd, response);
        return;
    }
    // Continue if correct password

    // Setup client object
    client->setUserName(username);
    client->setID(clientId);
    client->setNickName(nickname);

    response.setCommand(0x10); // Login successful
    Server::sendClient(client_sd, response);
}

// !register <username> <password> <nickname>
// TODO

// !msg <channel> <message>
void Server::channelMsg(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();

    Message response;
    response.setType(0x02);

    // Check if client authenticated
    if (!client->isAuthenticated())
    {
        response.setCommand(0x02); // Not Authenticated error
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if args exist
    if (args.size() < 2)
    {
        response.setCommand(0x00); // Client Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    std::string channel_name = args[0];

    // Channel name check
    if ((channel_name.size() < 2) or !channel_name.starts_with("#"))
    {
        response.setCommand(0x31); // Failed to send message
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if channel exists
    Channel *channel = getChannel(channel_name.substr(1));
    if (!channel)
    {
        response.setCommand(0x31); // Failed to send message
        Server::sendClient(client_sd, response);
        return;
    }

    // If the client not in the channel
    if (!channel->isMember(client->getID()))
    {
        response.setCommand(0x31); // Failed to send message
        Server::sendClient(client_sd, response);
        return;
    }

    int channel_id = channel->getId();

    std::string text = args[1];

    if (!this->db.insertChannelMessage(client->getID(), channel_id, text))
    {
        response.setCommand(0x01); // Server Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    // Broadcast
    Message toChannel;
    toChannel.setType(0x02);
    toChannel.setCommand(0x33);
    toChannel.addArg("1");
    toChannel.addArg("#" + channel->getName()); // Channel name
    toChannel.addArg(client->getUserName());    // Username
    toChannel.addArg(msg.getArgs()[1]);
    toChannel.addArg(date_time());

    std::vector<int> recipients;

    // Collect all clients who are members of the channel (excluding the sender)
    {
        std::lock_guard<std::mutex> lock(this->clientsMutex);

        for (auto &[client_id, client_ptr] : clients)
        {
            if (channel->isMember(client_ptr->getID()) and client_ptr->isAuthenticated() and client_sd != client_ptr->getClientfd())
            {
                recipients.push_back(client_id);
            }
        }
    }

    for (int recipient_id : recipients)
    {

        Client *recipient = Server::getClient(recipient_id);
        Server::sendClient(recipient->getClientfd(), toChannel);
    }

    // Response
    response.setCommand(0x30);
    Server::sendClient(client_sd, response);
}

// !getMsgC <channel> <page>
void Server::getChannelMsg(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();

    Message response;
    response.setType(0x02);

    // Check if client authenticated
    if (!client->isAuthenticated())
    {
        response.setCommand(0x02); // Not Authenticated error
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if args exist
    if (args.size() < 2)
    {
        response.setCommand(0x00); // Client Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    std::string channel_name = args[0];

    // Channel name check
    if ((channel_name.size() < 2) or !channel_name.starts_with("#"))
    {
        response.setCommand(0x34); // Failed to receive message
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if channel exists
    Channel *channel = getChannel(channel_name.substr(1));
    if (!channel)
    {
        response.setCommand(0x34); // Failed to receive message
        Server::sendClient(client_sd, response);
        return;
    }

    // If the client not in the channel
    if (!channel->isMember(client->getID()))
    {
        response.setCommand(0x34); // Failed to receive message
        Server::sendClient(client_sd, response);
        return;
    }

    int page = stoi(args[1]);

    std::vector<std::tuple<std::string, std::string, std::string>> messages;
    if (!this->db.getChannelMessagesPaginated(channel->getId(), page, messages))
    {
        response.setCommand(0x01); // Server Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    response.setCommand(0x33); // Message from channel
    response.addArg(std::to_string(messages.size()));
    for (const auto &msg : messages)
    {
        response.addArg("#" + channel->getName());
        response.addArg(std::get<0>(msg));
        response.addArg(std::get<1>(msg));
        response.addArg(std::get<2>(msg));
    }
    Server::sendClient(client_sd, response);
}

// !msg <user> <message>
void Server::userMsg(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();

    Message response;
    response.setType(0x02);

    // Check if client authenticated
    if (!client->isAuthenticated())
    {
        response.setCommand(0x02); // Not Authenticated error
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if args exist
    if (args.size() < 2)
    {
        response.setCommand(0x00); // Client Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    std::string username = args[0];

    // Check if client exists
    int recipient_id;
    if (!db.getClientByUsername(username, recipient_id))
    {
        response.setCommand(0x31); // Failed to send message
        Server::sendClient(client_sd, response);
        return;
    }

    std::string text = args[1];

    if (!this->db.insertPrivateMessage(client->getID(), recipient_id, text))
    {
        response.setCommand(0x01); // Server Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    int recipient_sd = -1;
    for (auto &[client_id, client_ptr] : clients)
    {
        if (client_ptr->getID() == recipient_id)
        {
            recipient_sd = client_ptr->getClientfd();
            break;
        }
    }

    if (recipient_sd == -1)
    {
        response.setCommand(0x30);
        Server::sendClient(client_sd, response);
        return;
    }

    // Broadcast
    Message toRecipient;
    toRecipient.setType(0x02);
    toRecipient.setCommand(0x32);
    toRecipient.addArg("1");
    toRecipient.addArg(client->getUserName()); // Sender name
    toRecipient.addArg(msg.getArgs()[1]);      // Message
    toRecipient.addArg(date_time());
    Server::sendClient(recipient_sd, toRecipient);

    // Response
    response.setCommand(0x30);
    Server::sendClient(client_sd, response);
}

// !getMsgU <page>
// !getMsgU <user> <page>
void Server::getUserMsg(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();

    Message response;
    response.setType(0x02);

    // Check if client authenticated
    if (!client->isAuthenticated())
    {
        response.setCommand(0x02); // Not Authenticated error
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if args exist
    if (args.size() < 1)
    {
        response.setCommand(0x00); // Client Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    int page;
    std::string username;
    std::vector<int> ids;

    if (args.size() > 1)
    {
        username = args[0];
        page = stoi(args[1]);

        int recipient_id;
        if (!db.getClientByUsername(username, recipient_id))
        {
            response.setCommand(0x34); // Failed to receive message
            Server::sendClient(client_sd, response);
            return;
        }

        ids.push_back(recipient_id);
    }
    else
    {
        page = stoi(args[0]);
        if (!this->db.getPrvMsgIds(client->getID(), ids))
        {
            response.setCommand(0x34); // Failed to receive message
            Server::sendClient(client_sd, response);
            return;
        }
    }

    std::vector<std::tuple<std::string, std::string, std::string>> messages;
    for (int &id : ids)
    {
        if (!this->db.getPrivateMessagesPaginated(client->getID(), id, page, messages))
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            continue;
        }

        response.setCommand(0x32); // Message from user
        response.clearArgs();
        response.addArg(std::to_string(messages.size()));

        for (const auto &msg : messages)
        {
            response.addArg(std::get<0>(msg));
            response.addArg(std::get<1>(msg));
            response.addArg(std::get<2>(msg));
        }
        Server::sendClient(client_sd, response);
    }
}

// !listc
void Server::listChannels(int client_sd, Message &msg)
{
    Message response;
    response.setType(0x02);

    Client *client = Server::getClient(client_sd);

    if (!client->isAuthenticated())
    {
        response.setCommand(0x02);
        Server::sendClient(client_sd, response);
        return;
    }

    response.setCommand(0x40);

    {
        std::lock_guard<std::mutex> lock(this->channelsMutex);
        for (const auto &pair : channels)
        {
            const auto &channel = pair.second;
            response.addArg(channel->getName());
        }
    }

    Server::sendClient(client_sd, response);
}

// !listu
void Server::listOnlineUsers(int client_sd, Message &msg)
{
    Message response;
    response.setType(0x02);

    Client *client = Server::getClient(client_sd);

    if (!client->isAuthenticated())
    {
        response.setCommand(0x02);
        Server::sendClient(client_sd, response);
        return;
    }

    response.setCommand(0x41);

    {
        std::lock_guard<std::mutex> lock(this->clientsMutex);
        for (const auto &pair : clients)
        {
            const auto &client = pair.second;
            response.addArg(client->getUserName());
        }
    }

    Server::sendClient(client_sd, response);
}

// !join <channel>
void Server::joinChannel(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();

    Message response;
    response.setType(0x02);

    // Check if client authenticated
    if (!client->isAuthenticated())
    {
        response.setCommand(0x02); // Not Authenticated error
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if args exist
    if (args.size() < 1)
    {
        response.setCommand(0x00); // Client Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    std::string channel_name = args[0];

    // Channel name check
    if ((channel_name.size() < 2) or !channel_name.starts_with("#"))
    {
        response.setCommand(0x51); // channel does not exist
        Server::sendClient(client_sd, response);
        return;
    }

    Channel *channel = getChannel(channel_name.substr(1));
    if (!channel)
    {
        response.setCommand(0x51); // channel does not exist
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if the client is already in the channel
    if (channel->isMember(client->getID()))
    {
        // Client is already in the channel
        response.setCommand(0x50); // joined channel
        response.addArg(channel_name);
        Server::sendClient(client_sd, response);
        return;
    }

    // TODO check for channel key

    // Add the client to the channel
    if (!channel->addMember(client->getID(), this->db))
    {
        response.setCommand(0x01); // server side error
        Server::sendClient(client_sd, response);
        return;
    }

    response.setCommand(0x50); // joined channel
    response.addArg(channel_name);
    Server::sendClient(client_sd, response);

    // TODO broadcast a join message to the channel
}

// Invalid/Unknown Command
void Server::invalidCommand(int client_sd)
{
    Message response;
    response.setType(0x02);
    response.setCommand(0x00); // Client Side Error
    Server::sendClient(client_sd, response);
    return;
}

// !send
void Server::upload(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();

    Message response;
    response.setType(0x02);

    // Check if client authenticated
    if (!client->isAuthenticated())
    {
        response.setCommand(0x02); // Not Authenticated error
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if args exist
    if (args.size() < 3)
    {
        response.setCommand(0x00); // Client Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    std::string name = args[0];
    std::string filename = args[1];
    std::string file_data = args[2];

    // Channel name check
    if (!name.starts_with("#"))
    {
        // Check if client exists
        int recipient_id;
        if (!db.getClientByUsername(name, recipient_id))
        {
            response.setCommand(0x72); // File upload failed
            Server::sendClient(client_sd, response);
            return;
        }

        std::string uid_file = Server::savefile(filename, file_data);
        if (uid_file.empty())
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            return;
        }

        if (!this->db.insertFile(filename, client->getID(), recipient_id, 0, uid_file))
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            return;
        }

        std::string text = "Sent a file " + filename + " -> " + uid_file;

        if (!this->db.insertPrivateMessage(client->getID(), recipient_id, text))
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            return;
        }

        int recipient_sd = -1;
        for (auto &[client_id, client_ptr] : clients)
        {
            if (client_ptr->getID() == recipient_id)
            {
                recipient_sd = client_ptr->getClientfd();
                break;
            }
        }

        if (recipient_sd == -1)
        {
            response.setCommand(0x70);
            Server::sendClient(client_sd, response);
            return;
        }

        // Broadcast
        Message toRecipient;
        toRecipient.setType(0x02);
        toRecipient.setCommand(0x32);
        toRecipient.addArg("1");
        toRecipient.addArg(client->getUserName()); // Sender name
        toRecipient.addArg(text);                  // Message
        toRecipient.addArg(date_time());
        Server::sendClient(recipient_sd, toRecipient);

        // Response
        response.setCommand(0x70);
        Server::sendClient(client_sd, response);
    }
    else
    {
        // Check if channel exists
        Channel *channel = getChannel(name.substr(1));
        if (!channel)
        {
            response.setCommand(0x72); // File upload failed
            Server::sendClient(client_sd, response);
            return;
        }

        // If the client not in the channel
        if (!channel->isMember(client->getID()))
        {
            response.setCommand(0x72); // File upload failed
            Server::sendClient(client_sd, response);
            return;
        }
        int channel_id = channel->getId();

        std::string uid_file = Server::savefile(filename, file_data);
        if (uid_file.empty())
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            return;
        }

        if (!this->db.insertFile(filename, client->getID(), 0, channel_id, uid_file))
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            return;
        }

        std::string text = "Sent a file " + filename + " -> " + uid_file;

        if (!this->db.insertChannelMessage(client->getID(), channel_id, text))
        {
            response.setCommand(0x01); // Server Side Error
            Server::sendClient(client_sd, response);
            return;
        }

        // Broadcast
        Message toChannel;
        toChannel.setType(0x02);
        toChannel.setCommand(0x33);
        toChannel.addArg("1");
        toChannel.addArg("#" + channel->getName()); // Channel name
        toChannel.addArg(client->getUserName());    // Username
        toChannel.addArg(text);
        toChannel.addArg(date_time());

        std::vector<int> recipients;

        // TODO Lock the channelsMutex and clientMutex to safely access the clients map

        // Collect all clients who are members of the channel (excluding the sender)
        for (auto &[client_id, client_ptr] : clients)
        {
            if (channel->isMember(client_ptr->getID()) and client_ptr->isAuthenticated() and client_sd != client_ptr->getClientfd())
            {
                recipients.push_back(client_id);
            }
        }

        for (int recipient_id : recipients)
        {
            Client *recipient = this->clients[recipient_id].get();
            Server::sendClient(recipient->getClientfd(), toChannel);
        }

        // Response
        response.setCommand(0x70);
        Server::sendClient(client_sd, response);
    }
}

void Server::download(int client_sd, Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::vector<std::string> args = msg.getArgs();

    Message response;
    response.setType(0x02);

    // Check if client authenticated
    if (!client->isAuthenticated())
    {
        response.setCommand(0x02); // Not Authenticated error
        Server::sendClient(client_sd, response);
        return;
    }

    // Check if args exist
    if (args.size() < 2)
    {
        response.setCommand(0x00); // Client Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    std::string name = args[0];
    std::string uuid = args[1];

    int recipientId, channelId;
    std::string filename;

    if (!db.getFileByUUID(uuid, filename, recipientId, channelId))
    {
        response.setCommand(0x73); // Server Side Error
        Server::sendClient(client_sd, response);
        return;
    }

    if (name.starts_with("#") and channelId)
    {
        Channel *channel = getChannel(name.substr(1));

        // If the client not in the channel
        if (!channel->isMember(client->getID()))
        {
            response.setCommand(0x73); // Failed to download file
            Server::sendClient(client_sd, response);
            return;
        }

        // File not in channel
        if (channel->getId() != channelId)
        {
            response.setCommand(0x73); // Failed to download file
            Server::sendClient(client_sd, response);
            return;
        }

        std::string fileContents;

        if (!Server::getfile(uuid, fileContents))
        {
            response.setCommand(0x73); // Failed to download file
            Server::sendClient(client_sd, response);
            return;
        }

        response.setCommand(0x71);
        response.addArg(filename);
        response.addArg(fileContents);
        Server::sendClient(client_sd, response);
        return;
    }
    else if (recipientId)
    {
        if (client->getID() != recipientId)
        {
            response.setCommand(0x73); // Failed to download file
            Server::sendClient(client_sd, response);
            return;
        }

        std::string fileContents;

        if (!Server::getfile(uuid, fileContents))
        {
            response.setCommand(0x73); // Failed to download file
            Server::sendClient(client_sd, response);
            return;
        }

        response.setCommand(0x71);
        response.addArg(filename);
        response.addArg(fileContents);
        Server::sendClient(client_sd, response);
        return;
    }
    else
    {
        response.setCommand(0x73); // Failed to download file
        Server::sendClient(client_sd, response);
        return;
    }
}

std::string Server::savefile(std::string filename, std::string contents)
{
    std::string directory = "./files";
    // Create the directory if it doesn't exist
    // if (!std::filesystem::exists(directory))
    // {
    //     std::filesystem::create_directory(directory); // Create the directory if it doesn't exist
    // }

    // Generate a unique ID
    std::string uniqueId = generateUniqueId();
    std::string newFilename = directory + "/file_" + uniqueId;
    std::ofstream file(newFilename);

    // Check if the file is opened successfully
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << newFilename << std::endl;
        return "";
    }

    // Write the contents to the file
    file << contents;

    // Check if writing succeeded
    if (!file)
    {
        std::cerr << "Error writing to file: " << newFilename << std::endl;
        return "";
    }

    file.close();
    return uniqueId;
}

bool Server::getfile(const std::string &uniqueId, std::string &contents)
{
    std::string directory = "./files";
    std::string filename = directory + "/file_" + uniqueId;
    std::ifstream file(filename);

    // Check if the file is opened successfully
    if (!file.is_open())
    {
        std::cerr << "Error opening file: " << filename << std::endl;
        return false; // Return false if the file couldn't be opened
    }

    // Use a stringstream to read the contents of the file
    std::stringstream buffer;
    buffer << file.rdbuf();

    // Check if reading succeeded
    if (!file)
    {
        std::cerr << "Error reading from file: " << filename << std::endl;
        return false;
    }

    // Close the file
    file.close();

    contents = buffer.str();
    return true;
}

std::string Server::generateUniqueId()
{
    // Get the current time in milliseconds
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

    // Random number generator
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    
    // Generate a random number
    int randomNum = dis(gen);

    // Combine the timestamp and random number to create a unique ID
    std::stringstream ss;
    ss << millis << "_" << randomNum;

    return ss.str();
}
