#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

void CommandHandler::handleTopic(Server& server,
                                 Client& client,
                                 const ParsedCommand& cmd)
{
    if (!requireRegistered(server, client))
        return;

    if (cmd.params.empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS,
                      client.getNickname() + " TOPIC",
                      "TOPIC requires a channel name")
        );
        return;
    }

    const std::string channelName = cmd.params[0];
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

    if (cmd.params.size() == 1)
    {
        if (channel->getTopic().empty())
        {
            server.sendMessage(
                client.getFd(),
                makeReply(RPL_NOTOPIC,
                          client.getNickname() + " " + channelName,
                          "No topic has been set for this channel")
            );
        }
        else
        {
            server.sendMessage(
                client.getFd(),
                makeReply(RPL_TOPIC,
                          client.getNickname() + " " + channelName,
                          channel->getTopic())
            );
        }

        return;
    }

    if (!channel->hasUser(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTONCHANNEL,
                      client.getNickname() + " " + channelName,
                      "You must join this channel before changing its topic")
        );
        return;
    }

    if (channel->isTopicProtected()
        && !channel->isOperator(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_CHANOPRIVSNEEDED,
                      client.getNickname() + " " + channelName,
                      "Only a channel operator may change the topic")
        );
        return;
    }

    channel->setTopic(cmd.params[1]);

    const std::string topicMessage =
        clientPrefix(client.getNickname(), client.getUsername())
        + " TOPIC " + channelName
        + " :" + channel->getTopic() + "\r\n";

    const std::vector<Client*>& members = channel->getUsers();

    for (size_t i = 0; i < members.size(); ++i)
        server.sendMessage(members[i]->getFd(), topicMessage);
}
