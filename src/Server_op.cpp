#include "../inc/Server.hpp"

/* PRIVATE ONES */
Client* Server::findClient(int fd)
{
    for (std::list<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (it->getFd() == fd)
            return &(*it);
    }
    return NULL;
}

pollfd* Server::findPollFd(int fd)
{
    for (size_t i = 0; i < _fds.size(); ++i)
    {
        if (_fds[i].fd == fd)
            return &_fds[i];
    }
    return NULL;
}
void Server::processBuffer(Client& client)
{
    std::string& buffer = client.getBuffer();
    size_t newline;

    while ((newline = buffer.find('\n')) != std::string::npos)
    {
        std::string line = buffer.substr(0, newline);
        buffer.erase(0, newline + 1);

        if (!line.empty() && line[line.size() - 1] == '\r')
            line.erase(line.size() - 1);
        if (line.empty())
            continue;

        ParsedCommand command = _parsed.parse(line);
        const int fd = client.getFd();
        _handler.execute(*this, client, command);
        if (!findClient(fd))
            return;
    }
}

void Server::flushClientOutput(int fd)
{
    Client* client = findClient(fd);
    pollfd* descriptor = findPollFd(fd);
    if (!client || !descriptor)
        return;

    while (client->hasPendingOutput())
    {
        std::string& output = client->getOutputBuffer();
        int sendFlags = 0;
        #ifdef MSG_NOSIGNAL
                sendFlags = MSG_NOSIGNAL;
        #endif
        ssize_t bytes = send(fd, output.c_str(), output.size(), sendFlags);
        if (bytes > 0)
        {
            client->consumeOutput(static_cast<size_t>(bytes));
            continue;
        }
        if (bytes < 0 && errno == EINTR)
            continue;
        if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            return;

        removeClient(fd);
        return;
    }

    descriptor = findPollFd(fd);
    if (descriptor)
        descriptor->events &= static_cast<short>(~POLLOUT);
}

void Server::removeClientFromChannels(Client* client)
{
    if (!client)
        return;

    size_t i = 0;
    while (i < _channels.size())
    {
        _channels[i].removeUser(client);
        if (_channels[i].getUserCount() == 0)
            _channels.erase(_channels.begin() + i);
        else
            ++i;
    }
}

/* PUBLIC ONES */
const std::string& Server::getPass() const
{
    return _pass;
}

void Server::sendMessage(int fd, const std::string& msg)
{
    Client* client = findClient(fd);
    pollfd* descriptor = findPollFd(fd);
    if (!client || !descriptor)
        return;

    client->appendOutput(msg);
    descriptor->events |= POLLOUT;
}

bool Server::isNickTaken(const std::string& wanted, int exceptFd) const
{
    for (std::list<Client>::const_iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (it->getFd() != exceptFd
            && !it->getNickname().empty()
            && ircCaseEqual(wanted, it->getNickname()))
            return true;
    }
    return false;
}

Client* Server::findClientByNick(const std::string& nickname)
{
    for (std::list<Client>::iterator it = _clients.begin();
         it != _clients.end(); ++it)
    {
        if (ircCaseEqual(it->getNickname(), nickname))
            return &(*it);
    }
    return NULL;
}


Channel* Server::findChannel(const std::string& name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (ircCaseEqual(_channels[i].getName(), name))
            return &_channels[i];
    }
    return NULL;
}

Channel* Server::createChannel(const std::string& name)
{
    _channels.push_back(Channel(name));
    return &_channels.back();
}

void Server::removeChannelIfEmpty(const std::string& name)
{
    for (size_t i = 0; i < _channels.size(); ++i)
    {
        if (ircCaseEqual(_channels[i].getName(), name)
            && _channels[i].getUserCount() == 0)
        {
            _channels.erase(_channels.begin() + i);
            return;
        }
    }
}


void Server::removeClientObject(int fd)
{
	for (std::list<Client>::iterator it = _clients.begin();
	it != _clients.end(); ++it)
    {
		if (it->getFd() == fd)
        {
			_clients.erase(it);
            return;
        }
    }
}

void Server::removePollFd(int fd)
{
	for (size_t i = 0; i < _fds.size(); ++i)
	{
		if (_fds[i].fd == fd)
		{
			_fds.erase(_fds.begin() + i);
			return;
		}
	}
}

void Server::removeClient(int fd)
{
	Client* client = findClient(fd);
    removeClientFromChannels(client);
    close(fd);
    removePollFd(fd);
    removeClientObject(fd);
    std::cout << "REMOVE fd = " << fd << std::endl;
}

void Server::quitClient(Client& client, const std::string& reason)
{
	std::vector<Client*> recipients;
	for (size_t i = 0; i < _channels.size(); ++i)
	{
		if (!_channels[i].hasUser(&client))
			continue;

		const std::vector<Client*>& members = _channels[i].getUsers();
		for (size_t j = 0; j < members.size(); ++j)
		{
			if (members[j] == &client)
				continue;

			bool alreadyAdded = false;
			for (size_t k = 0; k < recipients.size(); ++k)
			{
				if (recipients[k] == members[j])
				{
					alreadyAdded = true;
					break;
				}
			}
			if (!alreadyAdded)
				recipients.push_back(members[j]);
		}
	}
	const std::string quitMessage =
		clientPrefix(client.getNickname(), client.getUsername())
		+ " QUIT :" + reason + "\r\n";

	for (size_t i = 0; i < recipients.size(); ++i)
		sendMessage(recipients[i]->getFd(), quitMessage);

	removeClient(client.getFd());
}