#include "../inc/CommandHandler.hpp"
#include "../inc/Server.hpp"
#include "../inc/Channel.hpp"
#include "../inc/Macro.hpp"
#include "../inc/Utils.hpp"

#include <sstream>

bool CommandHandler::requireRegistered(Server& server, Client& client)
{
    if (client.isRegistered())
        return true;

    std::string target;

    if (client.hasNick())
        target = client.getNickname();
    else
        target = "*";

    server.sendMessage(
        client.getFd(),
        makeReply(ERR_NOTREGISTERED,
                  target,
                  "You have not registered")
    );

    return false;
}

void CommandHandler::handlePass(Server& server,
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
            makeReply(ERR_NEEDMOREPARAMS,
                      client.getNickname() + " JOIN",
                      "Not enough parameters")
        );
        return;
    }

    const std::string channelName = cmd.params[0];

    if (channelName.empty() || channelName[0] != '#')
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHCHANNEL,
                      client.getNickname() + " " + channelName,
                      "No such channel")
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

    if (!channelCreated && channel->isInviteOnly()
        && !channel->isInvited(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_INVITEONLYCHAN,
                      client.getNickname() + " " + channelName,
                      "Cannot join channel (+i)")
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
                makeReply(ERR_BADCHANNELKEY,
                          client.getNickname() + " " + channelName,
                          "Cannot join channel (+k)")
            );
            return;
        }
    }

    if (!channelCreated && channel->hasUserLimit()
        && channel->getUserCount() >= channel->getUserLimit())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_CHANNELISFULL,
                      client.getNickname() + " " + channelName,
                      "Cannot join channel (+l)")
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
            makeReply(RPL_NOTOPIC,
                      client.getNickname() + " " + channel->getName(),
                      "No topic is set")
        );
    }
    else
    {
        server.sendMessage(
            client.getFd(),
            makeReply(RPL_TOPIC,
                      client.getNickname() + " " + channel->getName(),
                      channel->getTopic())
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
        makeReply(RPL_NAMREPLY,
                  client.getNickname() + " = " + channel->getName(),
                  names)
    );

    server.sendMessage(
        client.getFd(),
        makeReply(RPL_ENDOFNAMES,
                  client.getNickname() + " " + channel->getName(),
                  "End of /NAMES list")
    );
}

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
                      "No recipient given (PRIVMSG)")
        );
        return;
    }

    if (cmd.params.size() < 2 || cmd.params[1].empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTEXTTOSEND,
                      client.getNickname(),
                      "No text to send")
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
                          "No such channel")
            );
            return;
        }

        if (!channel->hasUser(&client))
        {
            server.sendMessage(
                client.getFd(),
                makeReply(ERR_CANNOTSENDTOCHAN,
                          client.getNickname() + " " + target,
                          "Cannot send to channel")
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
                      "No such nick")
        );
        return;
    }

    server.sendMessage(receiver->getFd(), message);
}

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
                      "Not enough parameters")
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
                      "No such channel")
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
                          "No topic is set")
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
                      "You're not on that channel")
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
                      "You're not channel operator")
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

void CommandHandler::handleInvite(Server& server,
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
                      client.getNickname() + " INVITE",
                      "Not enough parameters")
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
                      "No such nick")
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
                      "No such channel")
        );
        return;
    }

    if (!channel->hasUser(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTONCHANNEL,
                      client.getNickname() + " " + channelName,
                      "You're not on that channel")
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

    if (channel->hasUser(invitedClient))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_USERONCHANNEL,
                      client.getNickname() + " "
                      + invitedNick + " " + channelName,
                      "is already on channel")
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
                      "Not enough parameters")
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
                      "No such channel")
        );
        return;
    }

    if (!channel->hasUser(&client))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOTONCHANNEL,
                      client.getNickname() + " " + channelName,
                      "You're not on that channel")
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

    Client* kickedClient = server.findClientByNick(kickedNick);

    if (!kickedClient)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHNICK,
                      client.getNickname() + " " + kickedNick,
                      "No such nick")
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
                      "They aren't on that channel")
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
                      "Not enough parameters")
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
                      "No such channel")
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
                      "You're not on that channel")
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
                          "is unknown mode char to me")
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
                              "No such nick")
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
                              "They aren't on that channel")
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

                std::istringstream limitStream(argument);
                size_t limit;
                char remainingCharacter;

                if (!(limitStream >> limit)
                    || limit == 0
                    || (limitStream >> remainingCharacter))
                {
                    continue;
                }

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
                          "is unknown mode char to me")
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
    else if (cmd.command == "TOPIC")
        handleTopic(server, client, cmd);
    else if (cmd.command == "INVITE")
        handleInvite(server, client, cmd);
    else if (cmd.command == "KICK")
        handleKick(server, client, cmd);
    else if (cmd.command == "MODE")
        handleMode(server, client, cmd);
    else
    {
        std::string target;

        if (client.hasNick())
            target = client.getNickname();
        else
            target = "*";

        server.sendMessage(
            client.getFd(),
            makeReply(ERR_UNKNOWNCOMMAND,
                      target + " " + cmd.command,
                      "Unknown command")
        );
    }
}