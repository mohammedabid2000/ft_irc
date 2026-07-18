#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"
#include "../../inc/Macro.hpp"
#include "../../inc/Utils.hpp"

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
        makeReply(
            ERR_NOTREGISTERED,
            target,
            "You have not registered, you need to complete registration before using this command"
        )
    );

    return false;
}