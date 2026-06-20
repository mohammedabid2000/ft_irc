#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <poll.h>
#include "Client.hpp"
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h> 
class Server
{
private:

    int _port;
    std::string _pass;
    int _serverSocket;
    std::vector<pollfd> _fds;
    std::vector<Client> _clients;

public:

    Server(int port, std::string pass);
    ~Server();
    void initSocket();        // socket + setsockopt
    void bindSocket();        // bind()
    void startListening();    // listen()
    void run();              // main loop
    void acceptClient();      // accept()
    void receiveData(int fd); // recv()
    void removeClient(int fd);
};

#endif