#include "../inc/Server.hpp"

#include <cstdlib>
#include <cerrno>
#include <climits>

static bool isValidPort(const char* value, int& port)
{
    if (!value || !value[0])
        return false;

    for (size_t i = 0; value[i]; ++i)
    {
        if (value[i] < '0' || value[i] > '9')
            return false;
    }

    errno = 0;
    char* end = NULL;
    long number = std::strtol(value, &end, 10);

    if (errno != 0 || !end || *end != '\0')
        return false;

    if (number < 1 || number > 65535)
        return false;

    port = static_cast<int>(number);
    return true;
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

    server.initSocket();
    server.bindSocket();
    server.startListening();
    server.run();

    return 0;
}