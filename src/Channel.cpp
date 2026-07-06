#include "../inc/Channel.hpp"

Channel::Channel()
{
}

Channel::Channel(const std::string& name)
    : _name(name)
{
}

Channel::~Channel()
{
}

std::string Channel::getName() const
{
    return _name;
}

void    Channel::addUser(Client* client)
{
    if(!client)
        return;
    if(hasUser(client))
        return;
    _users.push_back(client);
}

bool    Channel::hasUser(Client* client) const
{
    for(size_t i = 0; i < _users.size(); i++)
    {
        if(_users[i] == client)
            return true;
    }
    return false;
}

size_t  Channel::getUserCount() const
{
    return _users.size();
}

void    Channel::removeUser(Client* client)
{
    for(size_t i = 0; i < _users.size(); i++)
    {
        if(_users[i] == client)
        {
            _users.erase(_users.begin() + i);
            return;
        }
    }
}

const   std::vector<Client*>& Channel::getUsers() const
{
    return _users;
}