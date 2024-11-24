#include "client.hpp"

void Client::handleCommand(const std::string &data)
{
    // Remove the leading "!" from the command string
    std::string command = data.substr(1);
    std::istringstream stream(command);
    std::vector<std::string> tokens;

    // Tokenize the command string by spaces
    std::string token;
    while (std::getline(stream, token, ' '))
    {
        tokens.push_back(token);
    }

    if (tokens.empty())
    {
        std::cerr << "Empty command received" << std::endl;
        return;
    }

    // First token is the command
    std::string cmd = tokens[0];
    Message request;
    request.setType(0x01); // Default type for all messages

    if (cmd == "msg")
    {
        if (tokens.size() >= 3)
        {
            std::string recipient = tokens[1];                                                  // The second token is the recipient
            std::string message = command.substr(command.find(' ', command.find(' ') + 1) + 1); // Everything after the first space (recipient) is the message text

            if (recipient[0] == '#')
            {
                request.setCommand(0x30); // channels
            }
            else
            {
                request.setCommand(0x31); // individuals
            }
            request.addArg(recipient); // Add recipient
            request.addArg(message);   // Add message text
        }
        else
        {
            std::cerr << "Invalid command format for !msg" << std::endl;
            return;
        }
    }
    else if (cmd == "login")
    {
        // Expect two arguments: username and password
        if (tokens.size() >= 3)
        {
            std::string username = tokens[1];
            this->tmp = username;
            std::string password = tokens[2];
            request.setCommand(0x10); // Login command
            request.addArg(username); // Add username
            request.addArg(password); // Add password
        }
        else
        {
            std::cerr << "Invalid command format for !login" << std::endl;
            return;
        }
    }
    else if (cmd == "setpass")
    {
        // Expect one argument: new password
        if (tokens.size() >= 2)
        {
            std::string newPassword = tokens[1];
            request.setCommand(0x11);    // Set password command
            request.addArg(newPassword); // Add new password
        }
        else
        {
            std::cerr << "Invalid command format for !setpass" << std::endl;
            return;
        }
    }
    else if (cmd == "nick")
    {
        // Expect one argument: new nickname
        if (tokens.size() >= 2)
        {
            std::string nickname = tokens[1];
            request.setCommand(0x12); // Nickname command
            request.addArg(nickname); // Add nickname
        }
        else
        {
            std::cerr << "Invalid command format for !nick" << std::endl;
            return;
        }
    }
    else if (cmd == "listc")
    {
        // No arguments for listchannels
        request.setCommand(0x20); // List channels command
    }
    else if (cmd == "listu")
    {
        // No arguments for listusers
        request.setCommand(0x21); // List users command
    }
    else if (cmd == "getMsgU")
    {
        // Expect one argument: page (integer)
        if (tokens.size() == 2) // tokens[1] is page
        {
            int page = 0;

            try
            {
                page = std::stoi(tokens[1]); // Convert page to integer
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Invalid page number: " << tokens[1] << std::endl;
                return;
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "Page number out of range: " << tokens[1] << std::endl;
                return;
            }

            request.setCommand(0x22);             // Get user messages
            request.addArg(std::to_string(page)); // Add page number
        }
        // Expect two arguments: username (string) and page (integer)
        else if (tokens.size() == 3) // tokens[1] is username, tokens[2] is page
        {
            int page = 0;
            std::string username = tokens[1];
            try
            {
                page = std::stoi(tokens[2]); // Convert page to integer
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Invalid page number: " << tokens[1] << std::endl;
                return;
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "Page number out of range: " << tokens[1] << std::endl;
                return;
            }

            request.setCommand(0x22);             // Get user messages
            request.addArg(username);             // Add username name
            request.addArg(std::to_string(page)); // Add page number
        }
        else
        {
            std::cerr << "Invalid command format for !getMsgU. Expected: !getMsgU <username> <page>" << std::endl;
            return;
        }
    }
    else if (cmd == "getMsgC")
    {
        // Expect two arguments: channel name (string) and page (integer)
        if (tokens.size() >= 3) // tokens[1] is channel, tokens[2] is page
        {
            std::string channel = tokens[1];
            int page = 0;

            try
            {
                page = std::stoi(tokens[2]); // Convert page to integer
            }
            catch (const std::invalid_argument &e)
            {
                std::cerr << "Invalid page number: " << tokens[2] << std::endl;
                return;
            }
            catch (const std::out_of_range &e)
            {
                std::cerr << "Page number out of range: " << tokens[2] << std::endl;
                return;
            }

            request.setCommand(0x23);             // Get channel messages
            request.addArg(channel);              // Add channel name
            request.addArg(std::to_string(page)); // Add page number
        }
        else
        {
            std::cerr << "Invalid command format for !getMsgC. Expected: !getMsgC <channel> <page>" << std::endl;
            return;
        }
    }
    else if (cmd == "join")
    {
        // Expect one argument: channel name
        if (tokens.size() >= 2)
        {
            std::string channel = tokens[1];
            request.setCommand(0x40); // Join channel command
            request.addArg(channel);  // Add channel name
        }
        else
        {
            std::cerr << "Invalid command format for !join" << std::endl;
            return;
        }
    }
    else if (cmd == "change")
    {
        // Expect one argument: channel name
        if (tokens.size() >= 2)
        {
            std::string channel = tokens[1];
            request.setCommand(0x41); // Change channel command
            request.addArg(channel);  // channel name
        }
        else
        {
            std::cerr << "Invalid command format for !createchannel" << std::endl;
            return;
        }
    }
    else if (cmd == "create")
    {
        // Expect one argument: channel name
        if (tokens.size() >= 2)
        {
            std::string channel = tokens[1];
            request.setCommand(0x42); // Create channel command
            request.addArg(channel);  // Add channel name
        }
        else
        {
            std::cerr << "Invalid command format for !createchannel" << std::endl;
            return;
        }
    }
    else if (cmd == "desc")
    {
        // Expect two arguments: channel name and description
        if (tokens.size() >= 3)
        {
            std::string channel = tokens[1];
            std::string description = data.substr(data.find(channel) + channel.length() + 1); // The description after the channel
            request.setCommand(0x43);                                                         // Set channel description command
            request.addArg(channel);                                                          // Add channel name
            request.addArg(description);                                                      // Add description
        }
        else
        {
            std::cerr << "Invalid command format for !desc" << std::endl;
            return;
        }
    }
    else if (cmd == "setkey")
    {
        // Expect two arguments: channel name and key
        if (tokens.size() >= 3)
        {
            std::string channel = tokens[1];
            std::string key = data.substr(data.find(channel) + channel.length() + 1); // The key after the channel
            request.setCommand(0x44);                                                 // Set channel key command
            request.addArg(channel);                                                  // Add channel name
            request.addArg(key);                                                      // Add key
        }
        else
        {
            std::cerr << "Invalid command format for !setkey" << std::endl;
            return;
        }
    }
    else if (cmd == "kick")
    {
        // Expect two arguments: channel name and username
        if (tokens.size() >= 3)
        {
            std::string channel = tokens[1];
            std::string user = tokens[2];
            request.setCommand(0x50); // Kick user from channel
            request.addArg(channel);  // Add channel name
            request.addArg(user);     // Add username
        }
        else
        {
            std::cerr << "Invalid command format for !kick" << std::endl;
            return;
        }
    }
    else if (cmd == "ban")
    {
        // Expect two arguments: channel name and username
        if (tokens.size() >= 3)
        {
            std::string channel = tokens[1];
            std::string user = tokens[2];
            request.setCommand(0x51); // Ban user from channel
            request.addArg(channel);  // Add channel name
            request.addArg(user);     // Add username
        }
        else
        {
            std::cerr << "Invalid command format for !ban" << std::endl;
            return;
        }
    }
    else if (cmd == "ul")
    {
        if (tokens.size() == 3)
        {
            std::string recipient = tokens[1]; // The second token is the recipient (user or channel)
            std::string filename = tokens[2];  // The third token is the filename

            // Check if the file exists
            if (!std::filesystem::exists(filename))
            {
                std::cerr << "File does not exist: " << filename << std::endl;
                return;
            }

            // Open the file for reading
            std::ifstream file(filename, std::ios::binary);
            if (!file)
            {
                std::cerr << "Failed to open file: " << filename << std::endl;
                return;
            }

            // Read the first 512 bytes from the file
            std::vector<char> buffer(512);
            file.read(buffer.data(), buffer.size());

            // Check how many bytes were actually read
            std::streamsize bytesRead = file.gcount();
            if (bytesRead == 0)
            {
                std::cerr << "Failed to read from file or file is empty: " << filename << std::endl;
                return;
            }

            // Check for null bytes in the data
            bool containsNullByte = std::any_of(buffer.begin(), buffer.begin() + bytesRead, [](char c)
                                                { return c == '\0'; });
            if (containsNullByte)
            {
                std::cerr << "Invalid file: contains null byte(s)" << std::endl;
                return;
            }

            // Set the command for the request
            request.setCommand(0x60);

            // Add the recipient (user/channel) and filename
            request.addArg(recipient);
            request.addArg(filename);

            // Add the read data (up to 512 bytes)
            request.addArg(std::string(buffer.begin(), buffer.begin() + bytesRead));

            std::cout << "File data sent successfully." << std::endl;
        }
        else
        {
            std::cerr << "Invalid command format for !send" << std::endl;
            return;
        }
    }
    else if (cmd == "dl")
    {
        if (tokens.size() == 3)
        {
            std::string from = tokens[1]; // The second token is the (user or channel)
            std::string uid = tokens[2];  // The third token is the uuid

            // Set the command for the request
            request.setCommand(0x61);

            // Add the (user/channel) and uid
            request.addArg(from);
            request.addArg(uid);
        }
        else
        {
            std::cerr << "Invalid command format for !send" << std::endl;
            return;
        }
    }
    else
    {
        std::cerr << "Unknown command: " << data << std::endl;
        return;
    }

    Client::sendMessage(request);
}
