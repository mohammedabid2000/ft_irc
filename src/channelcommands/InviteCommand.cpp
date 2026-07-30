#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

void CommandHandler::handleInvite(Server& server,
                                  Client& client,
                                  const ParsedCommand& cmd)
{
    if (!requireRegistered(server, client))
        return;

    if (cmd.params.size() != 2)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS,
                      client.getNickname() + " INVITE",
                      "INVITE requires a nickname and a channel")
        );
        return;
    }

    const std::string invitedNick = cmd.params[0];
    const std::string channelName = cmd.params[1];

    Client* invitedClient = server.findClientByNick(invitedNick);

    if (!invitedClient)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHNICK,
                      client.getNickname() + " " + invitedNick,
                      "No connected user has this nickname")
        );
        return;
    }

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
                      "You must join this channel before inviting someone")
        );
        return;
    }

    if (!channel->isOperator(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_CHANOPRIVSNEEDED,
                      client.getNickname() + " " + channelName,
                      "Only a channel operator may invite users")
        );
        return;
    }

    if (channel->hasUser(invitedClient))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_USERONCHANNEL,
                      client.getNickname() + " "
                      + invitedNick + " " + channelName,
                      "is already a member of this channel")
        );
        return;
    }

    channel->addInvitedUser(invitedClient);

    server.sendMessage(
        client.getFd(),
        makeReply(RPL_INVITING,
                  client.getNickname() + " "
                  + invitedNick + " " + channelName,
                  "")
    );

    server.sendMessage(
        invitedClient->getFd(),
        clientPrefix(client.getNickname(), client.getUsername())
        + " INVITE " + invitedNick
        + " :" + channelName + "\r\n"
    );
}