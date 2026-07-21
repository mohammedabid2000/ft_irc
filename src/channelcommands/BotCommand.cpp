#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Utils.hpp"

#include <ctime>
#include <cctype>
#include <string>

static std::string toUpper(const std::string& text)
{
    std::string result = text;

    for (size_t i = 0; i < result.size(); ++i)
    {
        result[i] = static_cast<char>(
            std::toupper(static_cast<unsigned char>(result[i]))
        );
    }

    return result;
}

static std::string botPrefix()
{
    return ":Bot!bot@localhost";
}

static void sendBotMessage(Server& server,
                           Client& client,
                           const std::string& message)
{
    server.sendMessage(
        client.getFd(),
        botPrefix()
        + " PRIVMSG "
        + client.getNickname()
        + " :"
        + message
        + "\r\n"
    );
}

void CommandHandler::handleBot(Server& server,
                               Client& client,
                               const std::string& text)
{
    const std::string command = toUpper(text);

    if (command == "HELP")
    {
        sendBotMessage(
            server,
            client,
            "Available bot commands: HELP, TIME, ABOUT, COMMANDS"
        );
        return;
    }

    if (command == "TIME")
    {
        std::time_t currentTime = std::time(NULL);
        std::tm* localTime = std::localtime(&currentTime);

        if (!localTime)
        {
            sendBotMessage(
                server,
                client,
                "The server time is currently unavailable"
            );
            return;
        }

        char buffer[64];

        if (std::strftime(buffer,
                          sizeof(buffer),
                          "%Y-%m-%d %H:%M:%S",
                          localTime) == 0)
        {
            sendBotMessage(
                server,
                client,
                "The server time is currently unavailable"
            );
            return;
        }

        sendBotMessage(
            server,
            client,
            std::string("Current server time: ") + buffer
        );
        return;
    }

    if (command == "ABOUT")
    {
        sendBotMessage(
            server,
            client,
            "I am the ft_irc assistant bot"
        );
        return;
    }

    if (command == "COMMANDS")
    {
        sendBotMessage(
            server,
            client,
            "IRC commands: PASS, NICK, USER, JOIN, PRIVMSG, TOPIC, INVITE, KICK, MODE"
        );
        return;
    }

    sendBotMessage(
        server,
        client,
        "Unknown bot command. Send PRIVMSG Bot :HELP"
    );
}