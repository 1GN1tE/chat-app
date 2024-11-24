#include "client.hpp"

#include <iomanip> // Only for printing
#define ERROR "\033[31m[x]\033[0m "
#define INFO "\033[34m[!]\033[0m "

void Client::handleResponse(Message &response)
{
    int type = response.getType();
    int command = response.getCommand();
    std::vector<std::string> args = response.getArgs();

    switch (command)
    {
    case 0x00:
        std::cerr << ERROR << "Incorrect Request (Client Side Error)" << std::endl;
        break;

    case 0x01:
        std::cerr << ERROR << "Incorrect Request (Server Side Error)" << std::endl;
        break;

    case 0x02:
        std::cerr << ERROR << "Not Authenticated" << std::endl;
        break;

    // Login and User Management
    case 0x10:
        std::cerr << INFO << "Login successful" << std::endl;
        this->nick = this->tmp;
        break;

    case 0x11:
        std::cerr << INFO << "User created" << std::endl;
        break;

    case 0x12:
        std::cerr << ERROR << "Incorrect password" << std::endl;
        break;

    case 0x20:
        std::cerr << INFO << "Password updated successfully" << std::endl;
        break;

    case 0x21:
        std::cerr << ERROR << "Password update failed" << std::endl;
        break;

    case 0x22:
        std::cerr << INFO << "Nickname changed successfully" << std::endl;
        break;

    case 0x23:
        std::cerr << ERROR << "Nickname already in use" << std::endl;
        break;

    // Message
    case 0x30:
        std::cerr << INFO << "Message sent successfully" << std::endl;
        break;

    case 0x31:
        std::cerr << ERROR << "Failed to send message" << std::endl;
        break;

    case 0x32:
        Client::userMessage(response);
        break;

    case 0x33:
        Client::channelMessage(response);
        break;

    case 0x34:
        std::cerr << ERROR << "Failed to receive message" << std::endl;
        break;

    // Channel management
    case 0x40:
        std::cerr << INFO << "List of channels: " << std::endl;
        std::cout << "Channels: ";
        for (auto &ch : args)
        {
            std::cout << "#" << ch << " ";
        }
        std::cout << std::endl;
        break;

    case 0x41:
        std::cerr << INFO << "List of users: ";
        for (auto &ch : args)
        {
            std::cout << ch << " ";
        }
        std::cout << std::endl;
        break;

    case 0x50:
        std::cerr << INFO << "You have joined the channel" << std::endl;
        if (args.size() < 1)
        {
            std::cerr << "Args size is less than 1" << std::endl;
        }
        channel = args[0];
        break;

    case 0x51:
        std::cerr << ERROR << "Channel does not exist" << std::endl;
        break;

    case 0x52:
        std::cerr << INFO << "Channel created successfully" << std::endl;
        break;

    case 0x53:
        std::cerr << ERROR << "Channel already exists" << std::endl;
        break;

    case 0x54:
        std::cerr << INFO << "Topic changed to new topic" << std::endl;
        break;

    case 0x55:
        std::cerr << ERROR << "Permission denied (Changing topic)" << std::endl;
        break;

    case 0x56:
        std::cerr << ERROR << "Invalid topic format" << std::endl;
        break;

    case 0x57:
        std::cerr << INFO << "Channel key updated successfully" << std::endl;
        break;

    case 0x58:
        std::cerr << ERROR << "Permission denied (Changing channel key)" << std::endl;
        break;

    // User management in channel
    case 0x60:
        std::cerr << INFO << "User kicked from channel" << std::endl;
        break;

    case 0x61:
        std::cerr << ERROR << "User not found in channel" << std::endl;
        break;

    case 0x62:
        std::cerr << ERROR << "Permission denied" << std::endl;
        break;

    case 0x63:
        std::cerr << INFO << "User banned from channel" << std::endl;
        break;

    case 0x64:
        std::cerr << ERROR << "User not found in channel" << std::endl;
        break;

    case 0x65:
        std::cerr << ERROR << "Permission denied" << std::endl;
        break;

    // File transfer
    case 0x70:
        std::cerr << INFO << "File uploaded" << std::endl;
        break;

    case 0x71:
        std::cerr << INFO << "File downloaded" << std::endl;
        Client::saveToFile(response);
        break;

    case 0x72:
        std::cerr << ERROR << "File upload failed" << std::endl;
        break;

    case 0x73:
        std::cerr << ERROR << "File download failed" << std::endl;
        break;

    default:
        std::cerr << "Type: 0x" << std::setw(2) << std::setfill('0') << std::hex << (int)type
                  << ", Command: 0x" << std::setw(2) << std::setfill('0') << std::hex << (int)command
                  << ", Payload: ";

        for (auto arg : args)
        {
            std::cerr << arg << " ";
        }
        std::cerr << std::endl;

        break;
    }
}

void Client::channelMessage(const Message &msg)
{
    std::vector<std::string> args = msg.getArgs();

    // Check if there's at least one argument (the number of messages)
    if (args.empty())
    {
        std::cerr << "No arguments in the message." << std::endl;
        return;
    }

    // The first argument is the number of messages
    int numMessages = std::stoi(args[0]);

    // Ensure there are enough arguments for the number of messages
    if (args.size() != 1 + numMessages * 4)
    {
        std::cerr << "Invalid number of arguments. Expected " << (1 + numMessages * 4)
                  << " arguments for " << numMessages << " messages, got " << args.size() << "." << std::endl;
        return;
    }

    // Process each message
    for (int i = 0; i < numMessages; ++i)
    {
        int baseIndex = 1 + i * 4; // Calculate the starting index for each message's data

        // Check if we have enough arguments for the current message
        if (baseIndex + 3 >= args.size())
        {
            std::cerr << "Not enough arguments for message " << i + 1 << "." << std::endl;
            return;
        }

        // Extract the message details
        std::string channelName = args[baseIndex];   // Channel name
        std::string msgSender = args[baseIndex + 1]; // Message sender
        std::string message = args[baseIndex + 2];   // Message content
        std::string timestamp = args[baseIndex + 3]; // Timestamp

        // Display the message details
        std::cout << "Channel: " << channelName
                  << "  Sender: " << msgSender
                  << "  Message: " << message
                  << "  Timestamp: " << timestamp
                  << std::endl; // Print a blank line for readability
    }
}

void Client::userMessage(const Message &msg)
{
    std::vector<std::string> args = msg.getArgs();

    // Check if there's at least one argument (the number of messages)
    if (args.empty())
    {
        std::cerr << "No arguments in the message." << std::endl;
        return;
    }

    // The first argument is the number of messages
    int numMessages = std::stoi(args[0]);

    // Ensure there are enough arguments for the number of messages
    if (args.size() != 1 + numMessages * 3)
    {
        std::cerr << "Invalid number of arguments. Expected " << (1 + numMessages * 3)
                  << " arguments for " << numMessages << " messages, got " << args.size() << "." << std::endl;
        return;
    }

    // Process each message
    for (int i = 0; i < numMessages; ++i)
    {
        int baseIndex = 1 + i * 3; // Calculate the starting index for each message's data

        // Check if we have enough arguments for the current message
        if (baseIndex + 2 >= args.size())
        {
            std::cerr << "Not enough arguments for message " << i + 1 << "." << std::endl;
            return;
        }

        // Extract the message details
        std::string msgSender = args[baseIndex];     // Message sender
        std::string message = args[baseIndex + 1];   // Message content
        std::string timestamp = args[baseIndex + 2]; // Timestamp

        // Display the message details
        std::cout << "Sender: " << msgSender
                  << "  Message: " << message
                  << "  Timestamp: " << timestamp
                  << std::endl; // Print a blank line for readability
    }
}

void Client::saveToFile(const Message &msg)
{
    std::vector<std::string> args = msg.getArgs();

    // Check if there's at least one argument (the number of messages)
    if (args.size() != 2)
    {
        std::cerr << "Invalid arguments in the message." << std::endl;
        return;
    }

    std::string filename = args[0];
    std::string content = args[1];

    // Open the file in output mode (creates the file if it doesn't exist)
    std::ofstream file(filename);

    // Check if the file was successfully opened
    if (file.is_open())
    {
        // Write the content to the file
        file << content;

        // Close the file
        file.close();
        std::cout << "Content saved to " << filename << std::endl;
    }
    else
    {
        // If the file couldn't be opened, show an error message
        std::cerr << "Error opening file: " << filename << std::endl;
    }
}