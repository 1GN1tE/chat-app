#include "client.hpp"

Client::Client(const std::string &ip, int port)
    : IP(ip), port(port), clientSd(-1), stopReceiving(false), channel(""), user("") {}

Client::~Client()
{
    stopReceiving = true;
    if (receiveThread.joinable())
    {
        receiveThread.join();
    }
    if (clientSd >= 0)
    {
        close(clientSd);
    }
}

bool Client::connectToServer()
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(IP.c_str(), std::to_string(port).c_str(), &hints, &res);
    if (err != 0)
    {
        std::cerr << "Error resolving hostname: " << gai_strerror(err) << std::endl;
        return false;
    }

    clientSd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (clientSd < 0)
    {
        std::cerr << "Error creating socket!" << std::endl;
        freeaddrinfo(res);
        return false;
    }

    if (connect(clientSd, res->ai_addr, res->ai_addrlen) < 0)
    {
        std::cerr << "Error connecting to server!" << std::endl;
        close(clientSd);
        freeaddrinfo(res);
        return false;
    }

    freeaddrinfo(res);
    return true;
}

void Client::receiveMessages()
{
    std::vector<uint8_t> buffer(1024);
    ssize_t sz;

    while (!stopReceiving && (sz = recv(clientSd, buffer.data(), buffer.size(), 0)) > 0)
    {
        Message response;
        try
        {
            response = Message::deserialize(buffer);
        }
        catch (const std::exception &e)
        {
            std::cerr << "Deserialization Error: " << e.what() << std::endl;
            continue;
        }
        Client::handleResponse(response);
    }

    if (sz < 0 && !stopReceiving)
    {
        std::cerr << "Error receiving message!" << std::endl;
    }
}

void Client::sendMessageLoop()
{
    while (true)
    {
        std::string data;
        std::getline(std::cin, data);

        if (data == "!exit")
        {
            stopReceiving = true;
            shutdown(clientSd, SHUT_RDWR);
            close(clientSd); // Close the socket to unblock recv in receiveMessages
            break;
        }

        // Check if the first character is '!', then call handleCommand
        if (!data.empty() && data[0] == '!')
        {
            Message *msg = handleCommand(data);
            if (msg != NULL)
            {
                Client::sendMessage(*msg);
                delete msg;
            }
        }
        else
        {
            std::cerr << "Not a command" << std::endl;
        }
    }
}

void Client::sendMessage(Message &msg)
{
    std::vector<uint8_t> serialized = msg.serialize();
    ssize_t bytesSent = send(clientSd, serialized.data(), serialized.size(), 0);
    if (bytesSent < 0)
    {
        std::cerr << "Message not sent" << std::endl;
    }
}

void Client::run()
{
    if (!connectToServer())
    {
        return;
    }

    std::cout << "Connected to " << IP << " on port " << port << std::endl;

    try
    {
        receiveThread = std::thread(&Client::receiveMessages, this);
    }
    catch (const std::exception &e)
    {
        throw std::runtime_error("Error creating receive thread: " + std::string(e.what()));
    }

    sendMessageLoop();

    if (receiveThread.joinable())
    {
        receiveThread.join();
    }
}
