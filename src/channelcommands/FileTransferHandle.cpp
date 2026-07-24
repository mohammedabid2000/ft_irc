#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

static bool startsWith(const std::string& value, const std::string& prefix)
{
    return value.size() >= prefix.size()
        && value.compare(0, prefix.size(), prefix) == 0;
}

static bool isDccFileMessage(const std::string& text)
{
    const std::string dccPrefix = "\001DCC ";

    if (text.size() < dccPrefix.size() + 2
        || !startsWith(text, dccPrefix)
        || text[text.size() - 1] != '\001')
        return false;

    const std::string payload = text.substr(
        dccPrefix.size(), text.size() - dccPrefix.size() - 1
    );

    return startsWith(payload, "SEND ")
        || startsWith(payload, "RESUME ")
        || startsWith(payload, "ACCEPT ");
}

bool CommandHandler::handleFileTransfer(Server& server,
                                        Client& client,
                                        const std::string& target,
                                        const std::string& text)
{
    if (!target.empty() && target[0] == '#')
        return false;
    if (!isDccFileMessage(text))
        return false;

    Client* receiver = server.findClientByNick(target);

    if (!receiver)
    {
        server.sendMessage(
            client.getFd(),
            makeReply(ERR_NOSUCHNICK,
                      client.getNickname() + " " + target,
                      "No connected user has this nickname")
        );
        return true;
    }

    const std::string message =
        clientPrefix(client.getNickname(), client.getUsername())
        + " PRIVMSG " + receiver->getNickname() + " :" + text + "\r\n";

    server.sendMessage(receiver->getFd(), message);
    return true;
}
