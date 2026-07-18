#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel
{
    private:
        std::string _name;
        std::string _topic;
        std::vector<Client*> _users;
        std::vector<Client*> _operators;
        std::vector<Client*> _invitedUsers;

        bool _inviteOnly;
        bool _topicProtected;
        std::string _key;
        size_t _userLimit;

    public:
        Channel();
        Channel(const std::string& name);
        ~Channel();

        const std::string& getName() const;

        const std::string& getTopic() const;
        void setTopic(const std::string& newTopic);

        void addUser(Client* client);
        void removeUser(Client* client);
        bool hasUser(Client* client) const;

        void addOperator(Client* client);
        void removeOperator(Client* client);
        bool isOperator(Client* client) const;

        void addInvitedUser(Client* client);
        void removeInvitedUser(Client* client);
        bool isInvited(Client* client) const;

        void setInviteOnly(bool value);
        bool isInviteOnly() const;

        void setTopicProtected(bool value);
        bool isTopicProtected() const;

        void setKey(const std::string& key);
        void removeKey();
        bool hasKey() const;
        const std::string& getKey() const;

        void setUserLimit(size_t limit);
        void removeUserLimit();
        size_t getUserLimit() const;
        bool hasUserLimit() const;

        size_t getUserCount() const;
        const std::vector<Client*>& getUsers() const;
};

#endif