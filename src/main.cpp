#include <iostream>
#include <poll.h>
#include <unistd.h>

int main()
{
    pollfd fds[1];

    // stdin = keyboard input
    fds[0].fd = 0;

    // watch for input
    fds[0].events = POLLIN;

    while (true)
    {
        std::cout << "waiting...\n";

        int ret = poll(fds, 1, -1);

        if (fds[0].revents & POLLIN)
        {
            std::string line;

            std::getline(std::cin, line);

            std::cout << "you typed: "
                      << line
                      << std::endl;
        }
    }
}