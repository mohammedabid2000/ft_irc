#include "../inc/CommandHandler.hpp"
#include "../inc/Server.hpp"
#include "../inc/Macro.hpp"
#include "../inc/Utils.hpp"
void CommandHandler::handlePass(Server& server,
                               Client& client,
                               const ParsedCommand& cmd)
{
    // 1. already registered
    if (client.isRegistered())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_ALREADYREGISTERED, "*", "You may not reregister")
        );
        return;
    }

    // 2. missing password
    if (cmd.params.empty())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_PASS, "PASS", "Not enough parameters")
        );
        return;
    }

    // 3. extra params check (optional strict IRC behavior)
    if (cmd.params.size() > 1)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_PASS, "PASS", "Too many parameters")
        );
        return;
    }

    // 4. wrong password
    if (cmd.params[0] != server.getPass())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_PASSWDMISMATCH, "*", "Password incorrect")
        );
        return;
    }

    // server.sendMessage(client.getFd(),"Password accepeted\r\n");
        
    client.setPassAccepted(true);
}
void CommandHandler::handleNick(Server& server,
                               Client& client,
                               const ParsedCommand& cmd)
{
    if (cmd.params.empty())
    {
        server.sendMessage(client.getFd(), makeReply(ERR_NONICKNAMEGIVEN, "*", "No nickname given"));
        return;
    }

    std::string nick = cmd.params[0];
    if (!isValidNickname(nick))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_ERRONEUSNICKNAME, "*", "Erroneous nickname")
        );
        return;
    }
    if (server.isNickTaken(nick))
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NICKNAMEINUSE, "*", "Nickname is already in use")
        );
        return;
    }
    client.setNickname(nick);
    client.setHasNick(true);
    if (client.isRegistered())
    {
        std::string msg =
            ":" + std::string(SERVER_NAME) + " NICK :" + nick + "\r\n";

        server.sendMessage(client.getFd(), msg);
    }
    // server.sendMessage(client.getFd(),"Nickname set\r\n");
    tryRegister(server, client);
}
void CommandHandler::handleUser(Server& server,
                               Client& client,
                               const ParsedCommand& cmd)
{
    // 1. already registered
    if (client.isRegistered())
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_ALREADYREGISTERED, "*", "You may not reregister")
        );
        return;
    }

    // 2. missing params
    if (cmd.params.size() < 4)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NEEDMOREPARAMS_USER, "USER", "Not enough parameters")
        );
        return;
    }

    // 3. set user data
    client.setUsername(cmd.params[0]);
    client.setRealname(cmd.params[3]);
    client.setHasUser(true);
    std::cout << "Username: " << client.getUsername() << "Realname: "<< client.getRealname() << std::endl;
    // 4. attempt registration
    tryRegister(server,client);
    // server.sendMessage(client.getFd(),"User and real name set\r\n");
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

    std::string msg =
        ":ircserv 001 " +
        client.getNickname() +
        " :Welcome to ft_irc\r\n";

    server.sendMessage(client.getFd(), msg);
}
void CommandHandler::execute(Server& server,Client& client,  const ParsedCommand& cmd)
{
    if (cmd.command == "PASS")
        handlePass(server, client, cmd);

    else if (cmd.command == "NICK")
        handleNick(server, client, cmd);

    else if (cmd.command == "USER")
        handleUser(server,client, cmd);
    
    else if(cmd.command == "JOIN")
        handleJoin(server, client, cmd);
    
    else if(cmd.command == "PRIVMSG")
        handlePrivmsg(server, client, cmd);
        
    else
        std::cout << "Unknown command: "
                  << cmd.command
                  << std::endl;
}

void    CommandHandler::handleJoin(Server& server, Client& client, const ParsedCommand& cmd)
{
    (void)client;

    if(cmd.params.empty())
    {
        std::cout << "JOIN: missing channel name" << std::endl;
        return;
    }
    std::string channelName = cmd.params[0];
    Channel* channel = server.findChannel(channelName);
    if(!channel)
    {
        channel = server.createChannel(channelName);
        std::cout << "Channel created: " << channel->getName() << std::endl;
    }
    else
    {
        std::cout << "Channel exists: " << channel->getName() << std::endl;
    }
    if(channel->hasUser(&client))
    {
        server.sendMessage(client.getFd(), "You aare already in " + channel->getName() + "\r\n");
        return;
    }
    channel->addUser(&client);
    server.sendMessage(client.getFd(), "Joined " + channel->getName() + "\r\n");
    std::cout << client.getNickname() << " joined " << channel->getName() << std::endl;
    std::cout << "Users in channel: " << channel->getUserCount() << std::endl;
}
void CommandHandler::handlePrivmsg(Server& server, Client& client, const ParsedCommand& cmd)
{
    if (cmd.params.size() < 2)
    {
        server.sendMessage(client.getFd(), "PRIVMSG: not enough parameters\r\n");
        return;
    }

    std::string target = cmd.params[0];
    std::string message = cmd.params[1];

    Channel* channel = server.findChannel(target);
    if (!channel)
    {
        server.sendMessage(client.getFd(), "No such channel\r\n");
        return;
    }

    if (!channel->hasUser(&client))
    {
        server.sendMessage(client.getFd(), "You are not in this channel\r\n");
        return;
    }

    const std::vector<Client*>& users = channel->getUsers();

    for (size_t i = 0; i < users.size(); i++)
    {
        if (users[i] != &client)
        {
            server.sendMessage(users[i]->getFd(),":" + client.getNickname() + " PRIVMSG " + target + " :" + message + "\r\n");
        }
    }
}