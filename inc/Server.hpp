#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <poll.h>
#include "Client.hpp"
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <errno.h>
#include "CommandHandler.hpp"
#include <netdb.h>
#include "Channel.hpp"
class CommandHandler;
class Server
{
    private:
        int _port;
        std::string _pass;
        int _serverfd;
        std::vector<pollfd> _fds;
        std::vector<Client> _clients; //stores all clients
        std::vector<Channel> _channels; // to store all channels
        Parser _parsed;
        CommandHandler _handler;
        Client* findClient(int fd);
        void processBuffer(Client& client);
    public:
        Server(int port, std::string pass);
        ~Server();
        const std::string& getPass()const{return _pass;}
        void sendMessage(int fd, const std::string& msg);
        bool isNickTaken(std::string& wanted);
        void initSocket();        // socket + setsockopt
        void bindSocket();        // bind()
        void startListening();    // listen()
        void acceptClient();      // accept()
        void receiveData(int fd); // recv()
        void removeClient(int fd);     // global cleanup
        void removePollFd(int fd);         // remove from _fds
        void removeClientObject(int fd);   // remove from _clients
        void run();              // main loop
        Channel*    findChannel(const std::string& name);
        Channel*    createChannel(const std::string& name);
};

#endif