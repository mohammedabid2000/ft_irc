#include "../inc/Server.hpp"

Server::Server(int port, std::string pass):_port(port),_pass(pass)
{
    std::cout << "IRC server created\n";
}

Server::~Server()
{
    std::cout << "Server shutting down\n";
}

void    Server::initSocket()
{
    _serverfd  = socket(AF_INET, SOCK_STREAM, 0);
    if(_serverfd < 0)
        throw "Socket creation failed !\n";
    int yes = 1;
    setsockopt(_serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
    fcntl(_serverfd, F_SETFL, O_NONBLOCK);
}

void    Server::bindSocket()
{
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_port);
    addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(_serverfd,(const sockaddr*)&addr, sizeof(addr)) != 0)
        throw "Socket binding failed !\n";
}
void Server::startListening()
{
    listen(_serverfd, 10);
}
void Server::acceptClient()
{
    int c_fd = accept(_serverfd, NULL, NULL);
    if (c_fd < 0)
        throw "Accept failure !\n";
    fcntl(c_fd, F_SETFL, O_NONBLOCK);
    Client client(c_fd);
    _clients.push_back(client);
    pollfd clientPoll;
    clientPoll.fd = c_fd;
    clientPoll.events = POLLIN;
    _fds.push_back(clientPoll);
    std::cout << "new client connected\n";
}
Client* Server::findClient(int fd)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if (_clients[i].getFd() == fd)
            return (&_clients[i]);
    }
    return (NULL);
}
void Server::receiveData(int fd)
{
    char buffer[1025];
    int bytes = recv(fd, buffer, 1024, 0);
    if (bytes == 0)
    {
        std::cout << "client disconnected\n";
        removeClient(fd);
        return;
    }
    if (bytes < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        std::cout << "recv error\n";
        removeClient(fd);
        return;
    }
    buffer[bytes] = '\0';
    std::cout << "received from " << fd << ": " << buffer << std::endl; 
    Client *client = findClient(fd);
    client->appendBuffer(buffer);

}
    
void Server::removePollFd(int fd)
{
    for (size_t i = 0; i < _fds.size(); i++)
    {
        if(_fds[i].fd == fd)
        {
            _fds.erase(_fds.begin() + i);
            break;
        }
    } 
}
void Server::removeClientObject(int fd)
{
    for (size_t i = 0; i < _clients.size(); i++)
    {
        if(_clients[i].getFd() == fd)
        {   
            _clients.erase(_clients.begin() + i);
            break;
        }
    }
}
void Server::removeClient(int fd)
{
    close(fd);
    removePollFd(fd);
    removeClientObject(fd);
}
void Server::run()
{
    pollfd serverPoll;
    serverPoll.fd = _serverfd;
    serverPoll.events = POLLIN;
    _fds.push_back(serverPoll);
    while (true)
    {
        poll(&_fds[0], _fds.size(), -1);
        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_fds[i].fd == _serverfd)
                    acceptClient();
                else
                    receiveData(_fds[i].fd);
            }
        }
    }
}