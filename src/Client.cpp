#include "../inc/Client.hpp"
Client::Client():_fd(-1)
{
    std::cout << "Client Object created\n";
}
Client::Client(int fd):_fd(fd),_passAccepted(false), _hasNick(false),_hasUser(false),_registered(false)
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
std::string& Client::getBuffer()
{
    return _buffer;
}
void Client::clearBuffer()
{
    _buffer.clear();
}
/* getters and setters */
void Client::setNickname(const std::string& nick){_nickname = nick;}
std::string& Client::getNickname() {return _nickname;}
void Client::setUsername(const std::string& user){_username = user;}
std::string& Client::getUsername() {return _username;}
void Client::setRealname(const std::string& real){_realname = real;}
std::string& Client::getRealname() {return _realname;}
void Client::setPassAccepted(bool value){_passAccepted = value;}
bool Client::passAccepted() const{return _passAccepted;}
void Client::setHasNick(bool value){_hasNick = value;}
bool Client::hasNick() const{return _hasNick;}
void Client::setHasUser(bool value){_hasUser = value;}
bool Client::hasUser() const{return _hasUser;}
void Client::setRegistered(bool value){_registered = value;}
bool Client::isRegistered() const{return _registered;}
