#include "../inc/CommandHandler.hpp"
#include "../inc/Server.hpp"
#include "../inc/Macro.hpp"
#include "../inc/Utils.hpp"
void CommandHandler::handlePass(Server& server,
                               Client& client,
                               const ParsedCommand& cmd)
{
    // 1. already registered
    std::cout << "PASS handler called\n";
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

    server.sendMessage(client.getFd(),"Password accepeted\r\n");
        
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

    std::cout << "Entered NICK handler\n";
    if (!cmd.params.empty())
        std::cout << "Nick = [" << cmd.params[0] << "]\n";
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
    server.sendMessage(client.getFd(),"Nickname set\r\n");
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
    server.sendMessage(client.getFd(),"User and real name set\r\n");
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
    std::cout << "Command = [" << cmd.command << "]\n";
    if (cmd.command == "PASS")
        handlePass(server, client, cmd);

    else if (cmd.command == "NICK")
        handleNick(server, client, cmd);

    else if (cmd.command == "USER")
        handleUser(server,client, cmd);
    else
        std::cout << "Unknown command: "
                  << cmd.command
                  << std::endl;
}