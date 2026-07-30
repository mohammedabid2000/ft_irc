#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

void CommandHandler::handleJoin(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    if (!requireRegistered(server, client))
        return;

    if (cmd.params.empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(
                ERR_NEEDMOREPARAMS,
                client.getNickname() + " JOIN",
                "Not enough parameters, JOIN requires a channel name"
            )
        );
        return;
    }

    const std::string channelName = cmd.params[0];

    if (channelName.empty() || channelName[0] != '#')
    {
        server.sendMessage(
            client.getFd(),
            makeReply(
                ERR_NOSUCHCHANNEL,
                client.getNickname() + " " + channelName,
                "This channel does not exist"
            )
        );
        return;
    }

    Channel* channel = server.findChannel(channelName);
    bool channelCreated = false;

    if (!channel)
    {
        channel = server.createChannel(channelName);
        channelCreated = true;
    }

    if (!channel)
        return;

    if (channel->hasUser(&client))
        return;

    if (!channelCreated
        && channel->isInviteOnly()
        && !channel->isInvited(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(
                ERR_INVITEONLYCHAN,
                client.getNickname() + " " + channelName,
                "This channel is invite-only, you cannot join it"
            )
        );
        return;
    }

    if (!channelCreated && channel->hasKey())
    {
        if (cmd.params.size() < 2
            || cmd.params[1] != channel->getKey())
        {
            server.sendMessage(
                client.getFd(),
                makeReply(
                    ERR_BADCHANNELKEY,
                    client.getNickname() + " " + channelName,
                    "This channel key is missing or incorrect, you cannot join it"
                )
            );
            return;
        }
    }

    if (!channelCreated
        && channel->hasUserLimit()
        && channel->getUserCount() >= channel->getUserLimit())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(
                ERR_CHANNELISFULL,
                client.getNickname() + " " + channelName,
                "This channel has reached its user limit, you cannot join it"
            )
        );
        return;
    }

    channel->addUser(&client);

    if (channelCreated)
        channel->addOperator(&client);

    channel->removeInvitedUser(&client);

    const std::string joinMessage =
        clientPrefix(client.getNickname(), client.getUsername())
        + " JOIN :" + channel->getName() + "\r\n";

    const std::vector<Client*>& users = channel->getUsers();

    for (size_t i = 0; i < users.size(); ++i)
        server.sendMessage(users[i]->getFd(), joinMessage);

    if (channel->getTopic().empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(
                RPL_NOTOPIC,
                client.getNickname() + " " + channel->getName(),
                "No topic has been set for this channel"
            )
        );
    }
    else
    {
        server.sendMessage(
            client.getFd(),
            makeReply(
                RPL_TOPIC,
                client.getNickname() + " " + channel->getName(),
                channel->getTopic()
            )
        );
    }

    std::string names;

    for (size_t i = 0; i < users.size(); ++i)
    {
        if (!names.empty())
            names += " ";

        if (channel->isOperator(users[i]))
            names += "@";

        names += users[i]->getNickname();
    }

    server.sendMessage(
        client.getFd(),
        makeReply(
            RPL_NAMREPLY,
            client.getNickname() + " = " + channel->getName(),
            names
        )
    );

    server.sendMessage(
        client.getFd(),
        makeReply(
            RPL_ENDOFNAMES,
            client.getNickname() + " " + channel->getName(),
            "End of /NAMES list"
        )
    );
}
