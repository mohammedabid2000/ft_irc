#include "../inc/Client.hpp"

Client::Client()
    : _fd(-1), _passAccepted(false), _hasNick(false),
      _hasUser(false), _registered(false)
{
}

Client::Client(int fd)
    : _fd(fd), _passAccepted(false), _hasNick(false),
      _hasUser(false), _registered(false)
{
}

Client::~Client()
{
}

int Client::getFd() const
{
    return _fd;
}

void Client::appendBuffer(const std::string& data)
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

void Client::appendOutput(const std::string& data)
{
    _outputBuffer += data;
}

std::string& Client::getOutputBuffer()
{
    return _outputBuffer;
}

bool Client::hasPendingOutput() const
{
    return !_outputBuffer.empty();
}

void Client::consumeOutput(size_t count)
{
    _outputBuffer.erase(0, count);
}

void Client::setNickname(const std::string& nick)
{
    _nickname = nick;
}

std::string& Client::getNickname()
{
    return _nickname;
}

const std::string& Client::getNickname() const
{
    return _nickname;
}

void Client::setUsername(const std::string& user)
{
    _username = user;
}

std::string& Client::getUsername()
{
    return _username;
}

const std::string& Client::getUsername() const
{
    return _username;
}

void Client::setRealname(const std::string& real)
{
    _realname = real;
}

std::string& Client::getRealname()
{
    return _realname;
}

const std::string& Client::getRealname() const
{
    return _realname;
}

void Client::setPassAccepted(bool value)
{
    _passAccepted = value;
}

bool Client::passAccepted() const
{
    return _passAccepted;
}

void Client::setHasNick(bool value)
{
    _hasNick = value;
}

bool Client::hasNick() const
{
    return _hasNick;
}

void Client::setHasUser(bool value)
{
    _hasUser = value;
}

bool Client::hasUser() const
{
    return _hasUser;
}

void Client::setRegistered(bool value)
{
    _registered = value;
}

bool Client::isRegistered() const
{
    return _registered;
}
