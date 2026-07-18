#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <iostream>

class Client
{
    private:
        int _fd;
        std::string _buffer;
        std::string _outputBuffer;

        std::string _nickname;
        std::string _username;
        std::string _realname;

        bool _passAccepted;
        bool _hasNick;
        bool _hasUser;
        bool _registered;

    public:
        Client();
        Client(int fd);
        ~Client();

        int getFd() const;

        void appendBuffer(const std::string& data);
        std::string& getBuffer();
        void clearBuffer();

        void appendOutput(const std::string& data);
        std::string& getOutputBuffer();
        bool hasPendingOutput() const;
        void consumeOutput(size_t count);

        void setNickname(const std::string& nick);
        std::string& getNickname();
        const std::string& getNickname() const;

        void setUsername(const std::string& user);
        std::string& getUsername();
        const std::string& getUsername() const;

        void setRealname(const std::string& real);
        std::string& getRealname();
        const std::string& getRealname() const;

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
