#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
class Client
{
    private:

        int _fd;
        std::string _buffer;

    public:

        Client();
        Client(int fd);
        ~Client();
        int getFd() const;
        void appendBuffer(std::string data);
        std::string getBuffer() const;
};

#endif