#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>
class Client
{
    private:

        int _fd;
        std::string _buffer;
        // IRC identity
        std::string _nickname;
        std::string _username;
        std::string _realname;
        // Registration state
        bool _passAccepted;
        bool _hasNick;
        bool _hasUser;
        bool _registered;
    public:

        Client();
        Client(int fd);
        ~Client();
        int getFd() const;
        void appendBuffer(std::string data);
        std::string &getBuffer();
        void clearBuffer();
        /* getters and setters */
        void setNickname(const std::string& nick);
        std::string& getNickname();
        void setUsername(const std::string& user);
        std::string& getUsername();
        void setRealname(const std::string& real);
        std::string& getRealname();
        void setPassAccepted(bool value);
        bool passAccepted() const;
        void setHasNick(bool value);
        bool hasNick() const;
        void setHasUser(bool value);
        bool hasUser() const;
        void setRegistered(bool value);
        bool isRegistered() const;
};

#endif