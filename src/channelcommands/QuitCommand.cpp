#include "../../inc/CommandHandler.hpp"
#include "../../inc/Server.hpp"

void CommandHandler::handleQuit(Server& server,
                                Client& client,
                                const ParsedCommand& cmd)
{
    std::string reason = "Client Quit";

    if (!cmd.params.empty() && !cmd.params[0].empty())
        reason = cmd.params[0];

    server.quitClient(client, reason);
}
