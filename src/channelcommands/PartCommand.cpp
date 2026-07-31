#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

void CommandHandler::handlePart(Server& server,
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
                      client.getNickname() + " PART",
                      "PART requires a channel name")
        );
        return;
    }

    const std::string reason = (cmd.params.size() >= 2 && !cmd.params[1].empty()) ? cmd.params[1] : client.getNickname();
    const std::string channelName = cmd.params[0];
    Channel *channel = server.findChannel(channelName);
    if(!channel)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHCHANNEL,
                        client.getNickname() + " " + channelName,
                        "This channel does not exist"));
        return;
    }
    if (!channel->hasUser(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTONCHANNEL,
                        client.getNickname() + " " + channelName,
                        "You are not on this channel"));
        return;
    }
    const std::string partMessage = clientPrefix(client.getNickname(), client.getUsername())
                                    + " PART " + channel->getName() + " :" + reason + "\r\n";
    const std::vector<Client*>& members = channel->getUsers();
    for (size_t i = 0; i < members.size(); ++i)
        server.sendMessage(members[i]->getFd(), partMessage);
    channel->removeUser(&client);
    server.removeChannelIfEmpty(channelName);
}

