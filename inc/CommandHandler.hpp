#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP
#include "Client.hpp"
#include "Parser.hpp"
class Server;
class CommandHandler
{
    public:
        void execute(Server& server, Client& client, const ParsedCommand& cmd);
    private:
        void handlePass(Server& server, Client& client, const ParsedCommand& cmd);
        void handleNick(Server& server,Client& client, const ParsedCommand& cmd);
        void handleUser(Server& server, Client& client, const ParsedCommand& cmd);
        void tryRegister(Server& server,Client& client);
};
#endif