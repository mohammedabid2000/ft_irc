#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Channel.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

void CommandHandler::handleMode(Server& server,
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
                      client.getNickname() + " MODE",
                      "MODE is missing a required parameter")
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
        std::string modes = "+";
        std::string modeParameters;

        if (channel->isInviteOnly())
            modes += "i";

        if (channel->isTopicProtected())
            modes += "t";

        if (channel->hasKey())
        {
            modes += "k";
            modeParameters += " " + channel->getKey();
        }

        if (channel->hasUserLimit())
        {
            std::ostringstream limitStream;

            limitStream << channel->getUserLimit();

            modes += "l";
            modeParameters += " " + limitStream.str();
        }

        server.sendMessage(
            client.getFd(),
            makeReply(RPL_CHANNELMODEIS,
                      client.getNickname() + " "
                      + channelName + " "
                      + modes + modeParameters,
                      "")
        );

        return;
    }

    if (!channel->hasUser(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTONCHANNEL,
                      client.getNickname() + " " + channelName,
                      "You must join this channel before changing its modes")
        );
        return;
    }

    if (!channel->isOperator(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_CHANOPRIVSNEEDED,
                      client.getNickname() + " " + channelName,
                      "You're not channel operator")
        );
        return;
    }

    const std::string modeString = cmd.params[1];
    bool adding = true;
    bool signFound = false;
    size_t parameterIndex = 2;

    for (size_t i = 0; i < modeString.size(); ++i)
    {
        const char mode = modeString[i];

        if (mode == '+')
        {
            adding = true;
            signFound = true;
            continue;
        }

        if (mode == '-')
        {
            adding = false;
            signFound = true;
            continue;
        }

        if (!signFound)
        {
            server.sendMessage(
                client.getFd(),
                makeReply(ERR_UNKNOWNMODE,
                          client.getNickname() + " "
                          + std::string(1, mode),
                          "is not a supported channel mode")
            );
            continue;
        }

        std::string argument;
        bool modeChanged = false;

        if (mode == 'i')
        {
            channel->setInviteOnly(adding);
            modeChanged = true;
        }
        else if (mode == 't')
        {
            channel->setTopicProtected(adding);
            modeChanged = true;
        }
        else if (mode == 'k')
        {
            if (adding)
            {
                if (parameterIndex >= cmd.params.size())
                {
                    server.sendMessage(
                        client.getFd(),
                        makeReply(ERR_NEEDMOREPARAMS,
                                  client.getNickname() + " MODE",
                                  "Not enough parameters")
                    );
                    continue;
                }

                argument = cmd.params[parameterIndex];
                ++parameterIndex;

                if (argument.empty())
                    continue;

                channel->setKey(argument);
                modeChanged = true;
            }
            else
            {
                channel->removeKey();
                modeChanged = true;
            }
        }
        else if (mode == 'o')
        {
            if (parameterIndex >= cmd.params.size())
            {
                server.sendMessage(
                    client.getFd(),
                    makeReply(ERR_NEEDMOREPARAMS,
                              client.getNickname() + " MODE",
                              "Not enough parameters")
                );
                continue;
            }

            argument = cmd.params[parameterIndex];
            ++parameterIndex;

            Client* targetClient =
                server.findClientByNick(argument);

            if (!targetClient)
            {
                server.sendMessage(
                    client.getFd(),
                    makeReply(ERR_NOSUCHNICK,
                              client.getNickname() + " " + argument,
                              "No connected user has this nickname")
                );
                continue;
            }

            if (!channel->hasUser(targetClient))
            {
                server.sendMessage(
                    client.getFd(),
                    makeReply(ERR_USERNOTINCHANNEL,
                              client.getNickname() + " "
                              + argument + " " + channelName,
                              "This user is not a member of the channel")
                );
                continue;
            }

            if (adding)
                channel->addOperator(targetClient);
            else
                channel->removeOperator(targetClient);

            modeChanged = true;
        }
        else if (mode == 'l')
        {
            if (adding)
            {
                if (parameterIndex >= cmd.params.size())
                {
                    server.sendMessage(
                        client.getFd(),
                        makeReply(ERR_NEEDMOREPARAMS,
                                  client.getNickname() + " MODE",
                                  "Not enough parameters")
                    );
                    continue;
                }

                argument = cmd.params[parameterIndex];
                ++parameterIndex;
                bool validLimit = !argument.empty();
                for (size_t characterIndex = 0; characterIndex < argument.size(); ++characterIndex)
                {
                    if (argument[characterIndex] < '0'|| argument[characterIndex] > '9')
                    {
                        validLimit = false;
                        break;
                    }
                }
                std::istringstream limitStream(argument);
                size_t limit = 0;
                char remainingCharacter;
                if (!validLimit || !(limitStream >> limit) || limit == 0 || (limitStream >> remainingCharacter))
                    continue;
                channel->setUserLimit(limit);
                modeChanged = true;
            }
            else
            {
                channel->removeUserLimit();
                modeChanged = true;
            }
        }
        else
        {
            server.sendMessage(
                client.getFd(),
                makeReply(ERR_UNKNOWNMODE,
                          client.getNickname() + " "
                          + std::string(1, mode),
                          "is not a supported channel mode")
            );
            continue;
        }

        if (!modeChanged)
            continue;

        std::string modeMessage =
            clientPrefix(client.getNickname(),
                         client.getUsername())
            + " MODE " + channelName + " ";

        if (adding)
            modeMessage += "+";
        else
            modeMessage += "-";

        modeMessage += mode;

        if (!argument.empty())
            modeMessage += " " + argument;

        modeMessage += "\r\n";

        const std::vector<Client*>& members =
            channel->getUsers();

        for (size_t memberIndex = 0;
             memberIndex < members.size();
             ++memberIndex)
        {
            server.sendMessage(
                members[memberIndex]->getFd(),
                modeMessage
            );
        }
    }
}