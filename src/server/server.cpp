#include "server.hpp"

Server::Server(const std::string &_name, int port, int threadPoolSize, int dbPoolSize)
    : name(std::move(_name)), channels(), clients(), epoll_fd(-1), events(),
      db("chatapp.db", dbPoolSize), pool(threadPoolSize)
{
    createSocket(port);
}

Server::~Server()
{
    channels.clear();
    clients.clear();
}

void Server::createSocket(int port)
{
    sockaddr_in servAddr;
    memset((char *)&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_addr.s_addr = INADDR_ANY;
    servAddr.sin_port = htons(port);

    this->socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->socketfd < 0)
    {
        throwError("Error establishing the server socket");
    }

    int optval = 1;
    if (setsockopt(this->socketfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
    {
        close(this->socketfd);
        throwError("Error setting socket options");
    }

    if (bind(this->socketfd, (struct sockaddr *)&servAddr, sizeof(servAddr)) != 0)
    {
        close(this->socketfd);
        throwError("Error binding socket to local address");
    }

    if (listen(this->socketfd, 5) != 0)
    {
        close(this->socketfd);
        throwError("Error listening");
    }
}

void Server::initChannels(void)
{
    std::vector<int> channelIds;

    // Fetch all channel IDs
    if (this->db.getAllChannelIds(channelIds))
    {
        for (int channelId : channelIds)
        {
            std::string name, description, key;
            int creatorID;
            std::vector<int> members, admins;

            // Fetch details for each channel
            if (this->db.getChannel(channelId, name, description, key, creatorID, members, admins))
            {
                std::lock_guard<std::mutex> lock(channelsMutex);

                this->channels[channelId] = std::make_unique<Channel>(
                    channelId, name, description, key, creatorID, members, admins);
            }
            else
            {
                throwError("Failed to fetch channel data for channel ID " + std::to_string(channelId));
            }
        }
    }
    else
    {
        throwError("Failed to fetch channel IDs from the database.");
    }
}

void Server::startServer(void)
{
    initChannels();

    std::cout << "Server started" << std::endl;

    // Epoll instance
    this->epoll_fd = epoll_create1(0);
    if (this->epoll_fd == -1)
    {
        throwError("Error creating epoll instance");
    }

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET; // Edge-triggered mode
    ev.data.fd = this->socketfd;
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, this->socketfd, &ev) == -1)
    {
        throwError("Error adding server socket to epoll");
    }

    this->events.resize(16);

    while (true)
    {
        int n = epoll_wait(this->epoll_fd, this->events.data(), events.size(), -1);
        if (n == -1)
        {
            std::cerr << std::format("Error in epoll_wait: {}\n", std::strerror(errno));
            continue;
        }

        // Event loop
        for (int i = 0; i < n; ++i)
        {
            int fd = this->events[i].data.fd;

            if (fd == this->socketfd)
            {
                pool.enqueueTask([this]()
                                 { addClient(); });
            }
            else if (this->events[i].events & EPOLLIN)
            {
                pool.enqueueTask([this, fd]()
                                 { clientRequest(fd); });
            }
        }
    }

    close(this->epoll_fd);
}

void Server::addClient()
{
    sockaddr_in newSockAddr;
    socklen_t newSockAddrSize = sizeof(newSockAddr);
    int newSd = accept(this->socketfd, (struct sockaddr *)&newSockAddr, &newSockAddrSize);
    if (newSd < 0)
    {
        std::cerr << "Error accepting request from client!" << std::endl;
        return;
    }

    // Set the client socket to non-blocking
    if (fcntl(newSd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cerr << std::format("Error setting socket to non-blocking: {}\n", strerror(errno));
        close(newSd);
        return;
    }

    std::cout << std::format("[{}]: new connection from {} on socket {}\n", date_time(), inet_ntoa(newSockAddr.sin_addr), newSd);

    struct epoll_event clientEv;
    clientEv.events = EPOLLIN | EPOLLET;
    clientEv.data.fd = newSd;
    if (epoll_ctl(this->epoll_fd, EPOLL_CTL_ADD, newSd, &clientEv) == -1)
    {
        std::cerr << std::format("Error adding client socket to epoll: {}\n", strerror(errno));
        close(newSd);
        return;
    }

    {
        std::lock_guard<std::mutex> lock(this->clientsMutex);

        if (clients.size() == events.size())
        {
            events.resize(events.size() * 2);
        }

        auto client = std::make_unique<Client>(newSd, newSockAddr);

        this->clients[newSd] = std::move(client);
    }
}

void Server::removeClient(int client_sd)
{
    std::lock_guard<std::mutex> lock(this->clientsMutex);

    if (clients.find(client_sd) != clients.end())
    {
        clients.erase(client_sd);

        if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_sd, nullptr) == -1)
        {
            std::cerr << "Error removing client socket from epoll: " << strerror(errno) << std::endl;
        }
    }

    close(client_sd);
}

void Server::clientRequest(int client_sd)
{
    std::vector<uint8_t> buffer(1024);
    ssize_t sz = recv(client_sd, buffer.data(), buffer.size(), 0);

    if (sz == 0)
    {
        // Client disconnected
        std::cerr << std::format("[{}]: socket {} hung up\n", date_time(), client_sd);
        Server::removeClient(client_sd);
        return;
    }
    else if (sz < 0)
    {
        // recv() error occurred
        std::cerr << std::format("recv() error: {}\n", strerror(errno));
        Server::removeClient(client_sd);
        return;
    }

    Message msg;
    try
    {
        msg = Message::deserialize(buffer);
    }
    catch (const std::exception &e)
    {
        std::cerr << std::format("Deserialisation Error: {}\n", e.what());
        return;
    }

    if (msg.getType() != 0x01)
    {
        std::cout << std::format("{}: invalid request\n", client_sd);
        return;
    }

    std::cout << std::format("{}: Request Code: {}\n", client_sd, msg.getCommand());
    Server::handleRequest(client_sd, msg);
}

void Server::sendClient(int client_sd, const Message &msg)
{
    Client *client = Server::getClient(client_sd);
    std::lock_guard<std::mutex> lock(client->mutex);

    std::vector<uint8_t> requestData = msg.serialize();
    ssize_t bytesSent = send(client_sd, requestData.data(), requestData.size(), 0);
    if (bytesSent < 0)
    {
        std::cerr << "Error sending request to client" << std::endl;
    }
}

Channel *Server::getChannel(const std::string &channel_name)
{
    std::lock_guard<std::mutex> lock(this->channelsMutex);

    for (auto &pair : channels)
    {
        if (pair.second->getName() == channel_name)
        {
            return pair.second.get();
        }
    }
    return nullptr;
}

Channel *Server::getChannelById(int channel_id)
{
    std::lock_guard<std::mutex> lock(this->channelsMutex);

    for (auto &pair : channels)
    {
        if (pair.second->getId() == channel_id)
        {
            return pair.second.get();
        }
    }
    return nullptr;
}

Client *Server::getClient(int client_sd)
{
    std::lock_guard<std::mutex> lock(this->clientsMutex);
    return this->clients[client_sd].get();
}