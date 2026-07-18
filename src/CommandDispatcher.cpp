#include "../inc/CommandHandler.hpp"
#include "../inc/Server.hpp"
#include "../inc/Macro.hpp"
#include "../inc/Utils.hpp"

void CommandHandler::execute(Server& server,
                             Client& client,
                             const ParsedCommand& cmd)
{
    if (cmd.command.empty())
        return;

    if (cmd.command == "PASS")
        handlePass(server, client, cmd);
    else if (cmd.command == "NICK")
        handleNick(server, client, cmd);
    else if (cmd.command == "USER")
        handleUser(server, client, cmd);
    else if (cmd.command == "JOIN")
        handleJoin(server, client, cmd);
    else if (cmd.command == "PRIVMSG")
        handlePrivmsg(server, client, cmd);
    else if (cmd.command == "TOPIC")
        handleTopic(server, client, cmd);
    else if (cmd.command == "INVITE")
        handleInvite(server, client, cmd);
    else if (cmd.command == "KICK")
        handleKick(server, client, cmd);
    else if (cmd.command == "MODE")
        handleMode(server, client, cmd);
    else
    {
        std::string target;

        if (client.hasNick())
            target = client.getNickname();
        else
            target = "*";

        server.sendMessage(
            client.getFd(),
            makeReply(ERR_UNKNOWNCOMMAND,
                      target + " " + cmd.command,
                      "Command not recognized by this server")
        );
    }
}