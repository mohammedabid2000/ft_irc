#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include "Client.hpp"

class Channel 
{
private:
    std::string _name;
    std::vector<Client*> _users;

public:
    Channel();
    Channel(const std::string& name);
    ~Channel();

    std::string getName() const;

    void    addUser(Client* client);
    bool    hasUser(Client* client) const;
    size_t  getUserCount() const;
    
    void    removeUser(Client* client);
    const std::vector<Client*>& getUsers() const;

};

#endif