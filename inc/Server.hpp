#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <list>
#include <poll.h>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>

#include "Client.hpp"
#include "Parser.hpp"
#include "CommandHandler.hpp"
#include "Channel.hpp"
#include "Utils.hpp"
class CommandHandler;

class Server
{
    private:
        int _port;
        std::string _pass;
        int _serverfd;
        std::vector<pollfd> _fds;
        std::list<Client> _clients;
        std::vector<Channel> _channels;
        Parser _parsed;
        CommandHandler _handler;
        
        Client* findClient(int fd);
        pollfd* findPollFd(int fd);
        void processBuffer(Client& client);
        void flushClientOutput(int fd);
        void removeClientFromChannels(Client* client);

    public:
        Server(int port, const std::string& pass);
        ~Server();

        
        void initSocket();
        void bindSocket();
        void startListening();
        void acceptClient();
        void receiveData(int fd);
        void run();
        
        const std::string& getPass() const;
        void sendMessage(int fd, const std::string& msg);
        bool isNickTaken(const std::string& wanted, int exceptFd) const;
        Client* findClientByNick(const std::string& nickname);
        Channel* findChannel(const std::string& name);
        Channel* createChannel(const std::string& name);
        void removeChannelIfEmpty(const std::string& name);
        void removeClientObject(int fd);
        void removePollFd(int fd);
        void removeClient(int fd);
        void broadcastNickChange(Client& client, const std::string& oldNick, const std::string& newNick);
        void quitClient(Client& client, const std::string& reason);
};

#endif
