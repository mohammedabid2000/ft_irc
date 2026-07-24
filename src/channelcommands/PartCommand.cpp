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

    const std::string reason =
        (cmd.params.size() >= 2 && !cmd.params[1].empty())
        ? cmd.params[1] : client.getNickname();
    const std::string channelList = cmd.params[0];
    size_t start = 0;

    while (start <= channelList.size())
    {
        const size_t comma = channelList.find(',', start);
        const std::string channelName = channelList.substr(
            start,
            comma == std::string::npos ? std::string::npos : comma - start
        );
        Channel* channel = server.findChannel(channelName);

        if (!channel)
        {
            server.sendMessage(
                client.getFd(),
                makeReply(ERR_NOSUCHCHANNEL,
                          client.getNickname() + " " + channelName,
                          "This channel does not exist")
            );
        }
        else if (!channel->hasUser(&client))
        {
            server.sendMessage(
                client.getFd(),
                makeReply(ERR_NOTONCHANNEL,
                          client.getNickname() + " " + channelName,
                          "You are not on this channel")
            );
        }
        else
        {
            const std::string partMessage =
                clientPrefix(client.getNickname(), client.getUsername())
                + " PART " + channel->getName() + " :" + reason + "\r\n";
            const std::vector<Client*>& members = channel->getUsers();

            for (size_t i = 0; i < members.size(); ++i)
                server.sendMessage(members[i]->getFd(), partMessage);

            channel->removeUser(&client);
            server.removeChannelIfEmpty(channelName);
        }

        if (comma == std::string::npos)
            break;
        start = comma + 1;
    }
}
