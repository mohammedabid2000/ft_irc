#include "../inc/Channel.hpp"

void Channel::addUser(Client* client)
{
    if (client && !hasUser(client))
        _users.push_back(client);
}

void Channel::removeUser(Client* client)
{
    for (size_t i = 0; i < _users.size(); ++i)
    {
        if (_users[i] == client)
        {
            _users.erase(_users.begin() + i);
            break;
        }
    }

    removeOperator(client);
    removeInvitedUser(client);
}

bool Channel::hasUser(Client* client) const
{
    for (size_t i = 0; i < _users.size(); ++i)
    {
        if (_users[i] == client)
            return true;
    }

    return false;
}

void Channel::addOperator(Client* client)
{
    if (client && hasUser(client) && !isOperator(client))
        _operators.push_back(client);
}

void Channel::removeOperator(Client* client)
{
    for (size_t i = 0; i < _operators.size(); ++i)
    {
        if (_operators[i] == client)
        {
            _operators.erase(_operators.begin() + i);
            return;
        }
    }
}

bool Channel::isOperator(Client* client) const
{
    for (size_t i = 0; i < _operators.size(); ++i)
    {
        if (_operators[i] == client)
            return true;
    }

    return false;
}

void Channel::addInvitedUser(Client* client)
{
    if (client && !isInvited(client))
        _invitedUsers.push_back(client);
}

void Channel::removeInvitedUser(Client* client)
{
    for (size_t i = 0; i < _invitedUsers.size(); ++i)
    {
        if (_invitedUsers[i] == client)
        {
            _invitedUsers.erase(_invitedUsers.begin() + i);
            return;
        }
    }
}

bool Channel::isInvited(Client* client) const
{
    for (size_t i = 0; i < _invitedUsers.size(); ++i)
    {
        if (_invitedUsers[i] == client)
            return true;
    }

    return false;
}

size_t Channel::getUserCount() const
{
    return _users.size();
}

const std::vector<Client*>& Channel::getUsers() const
{
    return _users;
}