#include "../inc/Client.hpp"
Client::Client():_fd(-1)
{
    std::cout << "Client Object created\n";
}
Client::Client(int fd):_fd(fd)
{
    std::cout << "Client Object created with fd: " << fd << std::endl;
}
Client::~Client()
{
    std::cout << "Client Object destructor called\n"<< std::endl;
}
int Client::getFd() const{return _fd;}
void Client::appendBuffer(std::string data)
{
    _buffer += data;
}
std::string Client::getBuffer() const
{
    return (_buffer);
}
