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
    addr.sin_addr.s_addr = INADDR_ANY;

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
