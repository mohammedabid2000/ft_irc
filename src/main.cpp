#include "../inc/Server.hpp"
#include <cstdlib>

int main(int ac, char **av)
{
    if (ac != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>\n";
        return 1;
    }

    int port = std::atoi(av[1]);
    std::string password = av[2];

    try
    {
        Server server(port, password);
        server.initSocket();
        server.bindSocket();
        server.startListening();
        server.run();
    }
    catch (std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    return 0;
}