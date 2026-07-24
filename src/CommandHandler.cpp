#include "../inc/CommandHandler.hpp"
#include "../inc/Server.hpp"
#include "../inc/Macro.hpp"
#include "../inc/Utils.hpp"

#include <sstream>


void CommandHandler::handlePass(Server& server, Client& client, const ParsedCommand& cmd)
{
    if (client.isRegistered())
    {
        server.sendMessage(client.getFd(), makeReply(ERR_ALREADYREGISTERED,
                      client.getNickname(),"You may not reregister")
        );
        return;
    }

    if (cmd.params.empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_PASS,
                      "PASS",
                      "Not enough parameters")
        );
        return;
    }

    if (cmd.params.size() > 1)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_PASS,
                      "PASS",
                      "Too many parameters")
        );
        return;
    }

    if (cmd.params[0] != server.getPass())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_PASSWDMISMATCH,
                      "*",
                      "Password incorrect")
        );
        return;
    }

    client.setPassAccepted(true);
    tryRegister(server, client);
}

void CommandHandler::handleNick(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    std::string currentTarget;

    if (client.hasNick())
        currentTarget = client.getNickname();
    else
        currentTarget = "*";

    if (cmd.params.empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NONICKNAMEGIVEN,
                      currentTarget,
                      "No nickname given")
        );
        return;
    }

    const std::string newNick = cmd.params[0];

    if (!isValidNickname(newNick))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_ERRONEUSNICKNAME,
                      currentTarget + " " + newNick,
                      "Erroneous nickname")
        );
        return;
    }

    if (server.isNickTaken(newNick, client.getFd()))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NICKNAMEINUSE,
                      currentTarget + " " + newNick,
                      "Nickname is already in use")
        );
        return;
    }

    const std::string oldNick = client.getNickname();

    client.setNickname(newNick);
    client.setHasNick(true);

    if (client.isRegistered())
    {
        server.sendMessage(
            client.getFd(),
            clientPrefix(oldNick, client.getUsername())
            + " NICK :" + newNick + "\r\n"
        );
    }
    else
    {
        tryRegister(server, client);
    }
}

void CommandHandler::handleUser(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    if (client.isRegistered())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_ALREADYREGISTERED,
                      client.getNickname(),
                      "You may not reregister")
        );
        return;
    }

    if (cmd.params.size() < 4)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_USER,
                      "USER",
                      "Not enough parameters")
        );
        return;
    }

    client.setUsername(cmd.params[0]);
    client.setRealname(cmd.params[3]);
    client.setHasUser(true);

    tryRegister(server, client);
}

void CommandHandler::tryRegister(Server& server, Client& client)
{
    if (client.isRegistered())
        return;

    if (!client.passAccepted())
        return;

    if (!client.hasNick())
        return;

    if (!client.hasUser())
        return;

    client.setRegistered(true);

    server.sendMessage(
        client.getFd(),
        makeReply(
            RPL_WELCOME,
            client.getNickname(),
            "Welcome to ft_irc "
            + clientPrefix(client.getNickname(),
                           client.getUsername()).substr(1)
        )
    );
}