#ifndef COMMANDHANDLER_HPP
#define COMMANDHANDLER_HPP

#include "Client.hpp"
#include "Parser.hpp"

class Server;

class CommandHandler
{
    public:
        void execute(Server& server,
                     Client& client,
                     const ParsedCommand& cmd);

    private:
        void handlePass(Server& server,
                        Client& client,
                        const ParsedCommand& cmd);

        void handleNick(Server& server,
                        Client& client,
                        const ParsedCommand& cmd);

        void handleUser(Server& server,
                        Client& client,
                        const ParsedCommand& cmd);

        void handleJoin(Server& server,
                        Client& client,
                        const ParsedCommand& cmd);

        void handlePrivmsg(Server& server,
                           Client& client,
                           const ParsedCommand& cmd);

        void handleTopic(Server& server,
                         Client& client,
                         const ParsedCommand& cmd);

        void handleInvite(Server& server,
                          Client& client,
                          const ParsedCommand& cmd);

        void handleKick(Server& server,
                        Client& client,
                        const ParsedCommand& cmd);

        void handleMode(Server& server,
                        Client& client,
                        const ParsedCommand& cmd);

        void handleBot(Server& server,
                       Client& client,
                       const std::string& text);

        void tryRegister(Server& server, Client& client);
        bool requireRegistered(Server& server, Client& client);
};

#endif