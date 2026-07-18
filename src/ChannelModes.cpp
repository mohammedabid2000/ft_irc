#include "../inc/Channel.hpp"

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