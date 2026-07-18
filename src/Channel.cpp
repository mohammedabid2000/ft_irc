#include "../inc/Channel.hpp"

Channel::Channel()
    : _inviteOnly(false),
      _topicProtected(false),
      _userLimit(0)
{
}

Channel::Channel(const std::string& name)
    : _name(name),
      _inviteOnly(false),
      _topicProtected(false),
      _userLimit(0)
{
}

Channel::~Channel()
{
}

const std::string& Channel::getName() const
{
    return _name;
}

const std::string& Channel::getTopic() const
{
    return _topic;
}

void Channel::setTopic(const std::string& newTopic)
{
    _topic = newTopic;
}

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

void Channel::setInviteOnly(bool value)
{
    _inviteOnly = value;
}

bool Channel::isInviteOnly() const
{
    return _inviteOnly;
}

void Channel::setTopicProtected(bool value)
{
    _topicProtected = value;
}

bool Channel::isTopicProtected() const
{
    return _topicProtected;
}

void Channel::setKey(const std::string& key)
{
    _key = key;
}

void Channel::removeKey()
{
    _key.clear();
}

bool Channel::hasKey() const
{
    return !_key.empty();
}

const std::string& Channel::getKey() const
{
    return _key;
}

void Channel::setUserLimit(size_t limit)
{
    _userLimit = limit;
}

void Channel::removeUserLimit()
{
    _userLimit = 0;
}

size_t Channel::getUserLimit() const
{
    return _userLimit;
}

bool Channel::hasUserLimit() const
{
    return _userLimit != 0;
}

size_t Channel::getUserCount() const
{
    return _users.size();
}

const std::vector<Client*>& Channel::getUsers() const
{
    return _users;
}