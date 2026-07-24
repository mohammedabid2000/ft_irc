#include "../inc/Server.hpp"
#include "../inc/Utils.hpp"
#include <cstring>

Server::Server(int port, const std::string& pass)
    : _port(port), _pass(pass), _serverfd(-1)
{
    std::cout << "IRC server created" << std::endl;
}

Server::~Server()
{
    bool serverClosed = false;
    for (size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd == _serverfd)
            serverClosed = true;
        if (_fds[i].fd >= 0)
            close(_fds[i].fd);
    }
    if (_serverfd >= 0 && !serverClosed)
        close(_serverfd);
}

const std::string& Server::getPass() const
{
    return _pass;
}

void Server::initSocket()
{
    _serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverfd < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return;
    }

    int yes = 1;
    if (setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR,
                   &yes, sizeof(yes)) < 0)
    {
        std::cerr << "setsockopt failed" << std::endl;
        close(_serverfd);
        _serverfd = -1;
        return;
    }

    int flags = fcntl(_serverfd, F_GETFL, 0);
    if (flags < 0 || fcntl(_serverfd, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        std::cerr << "fcntl failed for listening socket" << std::endl;
        close(_serverfd);
        _serverfd = -1;
    }
}

void Server::bindSocket()
{
    if (_serverfd < 0)
        return;

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(_serverfd, reinterpret_cast<const sockaddr*>(&addr),
             sizeof(addr)) < 0)
    {
        std::cerr << "Socket binding failed" << std::endl;
        close(_serverfd);
        _serverfd = -1;
    }
}

void Server::startListening()
{
    if (_serverfd < 0)
        return;
    if (listen(_serverfd, 10) < 0)
    {
        std::cerr << "listen failed" << std::endl;
        close(_serverfd);
        _serverfd = -1;
    }
}

void Server::acceptClient()
{
    while (true)
    {
        int clientFd = accept(_serverfd, NULL, NULL);
        if (clientFd < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                return;
            if (errno == EINTR)
                continue;
            std::cerr << "accept failed" << std::endl;
            return;
        }

        int flags = fcntl(clientFd, F_GETFL, 0);
        if (flags < 0 || fcntl(clientFd, F_SETFL, flags | O_NONBLOCK) < 0)
        {
            std::cerr << "fcntl failed for client socket" << std::endl;
            close(clientFd);
            continue;
        }

        _clients.push_back(Client(clientFd));

        pollfd clientPoll;
        clientPoll.fd = clientFd;
        clientPoll.events = POLLIN;
        clientPoll.revents = 0;
        _fds.push_back(clientPoll);

        std::cout << "ACCEPT fd = " << clientFd << std::endl;
    }
}

Client* Server::findClient(int fd)
{
    for (std::list<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (it->getFd() == fd)
            return &(*it);
    }
    return NULL;
}

pollfd* Server::findPollFd(int fd)
{
    for (size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd == fd)
            return &_fds[i];
    }
    return NULL;
}

void Server::receiveData(int fd)
{
    Client* client = findClient(fd);
    if (!client)
        return;

    char buffer[4096];
    while (true)
    {
        ssize_t bytes = recv(fd, buffer, sizeof(buffer), 0);
        if (bytes > 0)
        {
            client->appendBuffer(std::string(buffer, static_cast<size_t>(bytes)));
            processBuffer(*client);
            if (!findClient(fd))
                return;
            continue;
        }
        if (bytes == 0)
        {
            removeClient(fd);
            return;
        }
        if (errno == EINTR)
            continue;
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        std::cerr << "recv failed on fd " << fd << std::endl;
        removeClient(fd);
        return;
    }
}

void Server::processBuffer(Client& client)
{
    std::string& buffer = client.getBuffer();
    size_t newline;

    while ((newline = buffer.find('\n')) != std::string::npos)
    {
        std::string line = buffer.substr(0, newline);
        buffer.erase(0, newline + 1);

        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            continue;

        ParsedCommand command = _parsed.parse(line);
        const int fd = client.getFd();
        _handler.execute(*this, client, command);
        if (!findClient(fd))
            return;
    }
}

void Server::sendMessage(int fd, const std::string& msg)
{
    Client* client = findClient(fd);
    pollfd* descriptor = findPollFd(fd);
    if (!client || !descriptor)
        return;

    client->appendOutput(msg);
    descriptor->events |= POLLOUT;
}

void Server::flushClientOutput(int fd)
{
    Client* client = findClient(fd);
    pollfd* descriptor = findPollFd(fd);
    if (!client || !descriptor)
        return;

    while (client->hasPendingOutput())
    {
        std::string& output = client->getOutputBuffer();
        int sendFlags = 0;
#ifdef MSG_NOSIGNAL
        sendFlags = MSG_NOSIGNAL;
#endif
        ssize_t bytes = send(fd, output.c_str(), output.size(), sendFlags);
        if (bytes > 0)
        {
            client->consumeOutput(static_cast<size_t>(bytes));
            continue;
        }
        if (bytes < 0 && errno == EINTR)
            continue;
        if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return;

        removeClient(fd);
        return;
    }

    descriptor = findPollFd(fd);
    if (descriptor)
        descriptor->events &= static_cast<short>(~POLLOUT);
}

void Server::removePollFd(int fd)
{
    for (size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd == fd)
        {
            _fds.erase(_fds.begin() + i);
            return;
        }
    }
}

void Server::removeClientFromChannels(Client* client)
{
    if (!client)
        return;

    size_t i = 0;
    while (i < _channels.size())
    {
        _channels[i].removeUser(client);
        if (_channels[i].getUserCount() == 0)
            _channels.erase(_channels.begin() + i);
        else
            ++i;
    }
}

void Server::removeClientObject(int fd)
{
    for (std::list<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (it->getFd() == fd)
        {
            _clients.erase(it);
            return;
        }
    }
}

void Server::removeClient(int fd)
{
    Client* client = findClient(fd);
    removeClientFromChannels(client);
    close(fd);
    removePollFd(fd);
    removeClientObject(fd);
    std::cout << "REMOVE fd = " << fd << std::endl;
}

void Server::run()
{
    if (_serverfd < 0)
        return;

    pollfd serverPoll;
    serverPoll.fd = _serverfd;
    serverPoll.events = POLLIN;
    serverPoll.revents = 0;
    _fds.push_back(serverPoll);

    while (true)
    {
        int ready = poll(&_fds[0], _fds.size(), -1);
        if (ready < 0)
        {
            if (errno == EINTR)
                continue;
            std::cerr << "poll failed" << std::endl;
            return;
        }

        size_t polledCount = _fds.size();
        for (size_t remaining = polledCount; remaining > 0; --remaining)
        {
            size_t i = remaining - 1;
            if (i >= _fds.size())
                continue;

            int fd = _fds[i].fd;
            short events = _fds[i].revents;
            if (events == 0)
                continue;

            if (fd == _serverfd)
            {
                if (events & POLLIN)
                    acceptClient();
                if (events & (POLLERR | POLLHUP | POLLNVAL))
                {
                    std::cerr << "Listening socket failure" << std::endl;
                    return;
                }
                continue;
            }

            if (events & POLLIN)
                receiveData(fd);
            if (!findClient(fd))
                continue;

            if (events & POLLOUT)
                flushClientOutput(fd);
            if (!findClient(fd))
                continue;

            if (events & (POLLERR | POLLHUP | POLLNVAL))
                removeClient(fd);
        }
    }
}

bool Server::isNickTaken(const std::string& wanted, int exceptFd) const
{
    for (std::list<Client>::const_iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (it->getFd() != exceptFd
            && !it->getNickname().empty()
            && ircCaseEqual(wanted, it->getNickname()))
            return true;
    }
    return false;
}

Client* Server::findClientByNick(const std::string& nickname)
{
    for (std::list<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (ircCaseEqual(it->getNickname(), nickname))
            return &(*it);
    }
    return NULL;
}

Channel* Server::findChannel(const std::string& name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (ircCaseEqual(_channels[i].getName(), name))
            return &_channels[i];
    }
    return NULL;
}

Channel* Server::createChannel(const std::string& name)
{
    _channels.push_back(Channel(name));
    return &_channels.back();
}

void Server::removeChannelIfEmpty(const std::string& name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (ircCaseEqual(_channels[i].getName(), name)
            && _channels[i].getUserCount() == 0)
        {
            _channels.erase(_channels.begin() + i);
            return;
        }
    }
}

void Server::quitClient(Client& client, const std::string& reason)
{
    std::vector<Client*> recipients;

    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (!_channels[i].hasUser(&client))
            continue;

        const std::vector<Client*>& members = _channels[i].getUsers();
        for (size_t j = 0; j < members.size(); ++j)
        {
            if (members[j] == &client)
                continue;

            bool alreadyAdded = false;
            for (size_t k = 0; k < recipients.size(); ++k)
            {
                if (recipients[k] == members[j])
                {
                    alreadyAdded = true;
                    break;
                }
            }
            if (!alreadyAdded)
                recipients.push_back(members[j]);
        }
    }

    const std::string quitMessage =
        clientPrefix(client.getNickname(), client.getUsername())
        + " QUIT :" + reason + "\r\n";

    for (size_t i = 0; i < recipients.size(); ++i)
        sendMessage(recipients[i]->getFd(), quitMessage);

    removeClient(client.getFd());
}
