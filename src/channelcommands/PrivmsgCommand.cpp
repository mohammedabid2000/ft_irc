#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

void CommandHandler::handlePrivmsg(Server& server,
                                   Client& client,
                                   const ParsedCommand& cmd)
{
    if (!requireRegistered(server, client))
        return;

    if (cmd.params.empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NORECIPIENT,
                      client.getNickname(),
                      "PRIVMSG requires a recipient")
        );
        return;
    }

    if (cmd.params.size() < 2 || cmd.params[1].empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTEXTTOSEND,
                      client.getNickname(),
                      "PRIVMSG requires a message")
        );
        return;
    }

    const std::string target = cmd.params[0];
    const std::string text = cmd.params[1];

    const std::string message =
        clientPrefix(client.getNickname(), client.getUsername())
        + " PRIVMSG " + target + " :" + text + "\r\n";

    if (!target.empty() && target[0] == '#')
    {
        Channel* channel = server.findChannel(target);

        if (!channel)
        {
            server.sendMessage(
                client.getFd(),
                makeReply(ERR_NOSUCHCHANNEL,
                          client.getNickname() + " " + target,
                          "This channel does not exist")
            );
            return;
        }

        if (!channel->hasUser(&client))
        {
            server.sendMessage(
                client.getFd(),
                makeReply(ERR_CANNOTSENDTOCHAN,
                          client.getNickname() + " " + target,
                          "You must join this channel before sending messages")
            );
            return;
        }

        const std::vector<Client*>& members = channel->getUsers();

        for (size_t i = 0; i < members.size(); ++i)
        {
            if (members[i] != &client)
                server.sendMessage(members[i]->getFd(), message);
        }

        return;
    }

    Client* receiver = server.findClientByNick(target);

    if (!receiver)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHNICK,
                      client.getNickname() + " " + target,
                      "No connected user has this nickname")
        );
        return;
    }

    server.sendMessage(receiver->getFd(), message);
}