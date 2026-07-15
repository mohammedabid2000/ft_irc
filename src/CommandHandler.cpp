#include "../inc/CommandHandler.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Macro.hpp"
#include "../inc/Utils.hpp"

bool CommandHandler::requireRegistered(Server& server, Client& client)
{
    if (client.isRegistered())
        return true;

    std::string target = client.hasNick() ? client.getNickname() : "*";
    server.sendMessage(client.getFd(),
        makeReply(ERR_NOTREGISTERED, target, "You have not registered"));
    return false;
}

void CommandHandler::handlePass(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    if (client.isRegistered())
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_ALREADYREGISTERED, client.getNickname(),
                      "You may not reregister"));
        return;
    }

    if (cmd.params.empty())
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_PASS, "PASS",
                      "Not enough parameters"));
        return;
    }

    if (cmd.params.size() > 1)
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_PASS, "PASS",
                      "Too many parameters"));
        return;
    }

    if (cmd.params[0] != server.getPass())
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_PASSWDMISMATCH, "*", "Password incorrect"));
        return;
    }

    client.setPassAccepted(true);
    tryRegister(server, client);
}

void CommandHandler::handleNick(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    std::string currentTarget = client.hasNick() ? client.getNickname() : "*";

    if (cmd.params.empty())
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NONICKNAMEGIVEN, currentTarget,
                      "No nickname given"));
        return;
    }

    const std::string nick = cmd.params[0];
    if (!isValidNickname(nick))
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_ERRONEUSNICKNAME, currentTarget + " " + nick,
                      "Erroneous nickname"));
        return;
    }

    if (server.isNickTaken(nick, client.getFd()))
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NICKNAMEINUSE, currentTarget + " " + nick,
                      "Nickname is already in use"));
        return;
    }

    const std::string oldNick = client.getNickname();
    client.setNickname(nick);
    client.setHasNick(true);

    if (client.isRegistered())
    {
        server.sendMessage(client.getFd(),
            clientPrefix(oldNick, client.getUsername())
            + " NICK :" + nick + "\r\n");
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
        server.sendMessage(client.getFd(),
            makeReply(ERR_ALREADYREGISTERED, client.getNickname(),
                      "You may not reregister"));
        return;
    }

    if (cmd.params.size() < 4)
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_USER, "USER",
                      "Not enough parameters"));
        return;
    }

    client.setUsername(cmd.params[0]);
    client.setRealname(cmd.params[3]);
    client.setHasUser(true);
    tryRegister(server, client);
}

void CommandHandler::tryRegister(Server& server, Client& client)
{
    if (client.isRegistered()
        || !client.passAccepted()
        || !client.hasNick()
        || !client.hasUser())
        return;

    client.setRegistered(true);
    server.sendMessage(client.getFd(),
        makeReply(RPL_WELCOME, client.getNickname(),
                  "Welcome to ft_irc "
                  + clientPrefix(client.getNickname(), client.getUsername()).substr(1)));
}

void CommandHandler::handleJoin(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    if (!requireRegistered(server, client))
        return;

    if (cmd.params.empty())
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS, client.getNickname() + " JOIN",
                      "Not enough parameters"));
        return;
    }

    const std::string channelName = cmd.params[0];
    if (channelName.empty() || channelName[0] != '#')
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NOSUCHCHANNEL,
                      client.getNickname() + " " + channelName,
                      "No such channel"));
        return;
    }

    Channel* channel = server.findChannel(channelName);
    if (!channel)
        channel = server.createChannel(channelName);
    if (!channel || channel->hasUser(&client))
        return;

    channel->addUser(&client);

    const std::string joinMessage =
        clientPrefix(client.getNickname(), client.getUsername())
        + " JOIN :" + channel->getName() + "\r\n";

    const std::vector<Client*>& users = channel->getUsers();
    for (size_t i = 0; i < users.size(); ++i)
        server.sendMessage(users[i]->getFd(), joinMessage);

    server.sendMessage(client.getFd(),
        makeReply(RPL_NOTOPIC,
                  client.getNickname() + " " + channel->getName(),
                  "No topic is set"));

    std::string names;
    for (size_t i = 0; i < users.size(); ++i)
    {
        if (!names.empty())
            names += " ";
        names += users[i]->getNickname();
    }

    server.sendMessage(client.getFd(),
        makeReply(RPL_NAMREPLY,
                  client.getNickname() + " = " + channel->getName(),
                  names));
    server.sendMessage(client.getFd(),
        makeReply(RPL_ENDOFNAMES,
                  client.getNickname() + " " + channel->getName(),
                  "End of /NAMES list"));
}

void CommandHandler::handlePrivmsg(Server& server,
                                   Client& client,
                                   const ParsedCommand& cmd)
{
    if (!requireRegistered(server, client))
        return;

    if (cmd.params.empty())
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NORECIPIENT, client.getNickname(),
                      "No recipient given (PRIVMSG)"));
        return;
    }
    if (cmd.params.size() < 2 || cmd.params[1].empty())
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NOTEXTTOSEND, client.getNickname(),
                      "No text to send"));
        return;
    }

    const std::string target = cmd.params[0];
    const std::string message = cmd.params[1];
    const std::string wireMessage =
        clientPrefix(client.getNickname(), client.getUsername())
        + " PRIVMSG " + target + " :" + message + "\r\n";

    if (!target.empty() && target[0] == '#')
    {
        Channel* channel = server.findChannel(target);
        if (!channel)
        {
            server.sendMessage(client.getFd(),
                makeReply(ERR_NOSUCHCHANNEL,
                          client.getNickname() + " " + target,
                          "No such channel"));
            return;
        }
        if (!channel->hasUser(&client))
        {
            server.sendMessage(client.getFd(),
                makeReply(ERR_CANNOTSENDTOCHAN,
                          client.getNickname() + " " + target,
                          "Cannot send to channel"));
            return;
        }

        const std::vector<Client*>& users = channel->getUsers();
        for (size_t i = 0; i < users.size(); ++i)
        {
            if (users[i] != &client)
                server.sendMessage(users[i]->getFd(), wireMessage);
        }
        return;
    }

    Client* receiver = server.findClientByNick(target);
    if (!receiver)
    {
        server.sendMessage(client.getFd(),
            makeReply(ERR_NOSUCHNICK,
                      client.getNickname() + " " + target,
                      "No such nick"));
        return;
    }

    server.sendMessage(receiver->getFd(), wireMessage);
}

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
    else
    {
        std::string target = client.hasNick() ? client.getNickname() : "*";
        server.sendMessage(client.getFd(),
            makeReply(ERR_UNKNOWNCOMMAND, target + " " + cmd.command,
                      "Unknown command"));
    }
}
