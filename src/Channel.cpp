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