#include "../inc/Server.hpp"

void do_everything(Server &server)
{
    server.initSocket();
    server.bindSocket();
    server.startListening();
    server.run();
}

int main(int ac, char** av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }
    int port;
    if (!isValidPort(av[1], port))
    {
        std::cerr << "Error: port must be a number between 1 and 65535"
                  << std::endl;
        return 1;
    }
    const std::string password = av[2];
    if (password.empty())
    {
        std::cerr << "Error: password cannot be empty" << std::endl;
        return 1;
    }
    Server server(port, password);
    do_everything(server);
    return 0;
}