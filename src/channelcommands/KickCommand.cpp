#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

void CommandHandler::handleKick(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    if (!requireRegistered(server, client))
        return;

    if (cmd.params.size() < 2)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS,
                      client.getNickname() + " KICK",
                      "KICK requires a channel and a nickname")
        );
        return;
    }

    const std::string channelName = cmd.params[0];
    const std::string kickedNick = cmd.params[1];

    Channel* channel = server.findChannel(channelName);

    if (!channel)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHCHANNEL,
                      client.getNickname() + " " + channelName,
                      "This channel does not exist")
        );
        return;
    }

    if (!channel->hasUser(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTONCHANNEL,
                      client.getNickname() + " " + channelName,
                      "You must join this channel before removing a user")
        );
        return;
    }

    if (!channel->isOperator(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_CHANOPRIVSNEEDED,
                      client.getNickname() + " " + channelName,
                      "Only a channel operator may remove users")
        );
        return;
    }

    Client* kickedClient = server.findClientByNick(kickedNick);

    if (!kickedClient)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHNICK,
                      client.getNickname() + " " + kickedNick,
                      "No connected user has this nickname")
        );
        return;
    }

    if (!channel->hasUser(kickedClient))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_USERNOTINCHANNEL,
                      client.getNickname() + " "
                      + kickedNick + " " + channelName,
                      "This user is not a member of the channel")
        );
        return;
    }

    std::string reason = client.getNickname();

    if (cmd.params.size() >= 3 && !cmd.params[2].empty())
        reason = cmd.params[2];

    const std::string kickMessage =
        clientPrefix(client.getNickname(), client.getUsername())
        + " KICK " + channelName + " "
        + kickedNick + " :" + reason + "\r\n";

    const std::vector<Client*>& members = channel->getUsers();

    for (size_t i = 0; i < members.size(); ++i)
        server.sendMessage(members[i]->getFd(), kickMessage);

    channel->removeUser(kickedClient);
}