#include "../inc/Server.hpp"

Server::Server(int port, std::string pass):_port(port),_pass(pass)
{
    std::cout << "IRC server created\n";
}

Server::~Server()
{
    std::cout << "Server shutting down\n";
}

void    Server::initSocket()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
        throw "Socket creation error\n";
    
}
