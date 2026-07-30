*This project has been created as part of the 42 curriculum by mohabid, sihfadil.*

# ft_irc

## Description

**ft_irc** is an Internet Relay Chat (IRC) server developed in **C++98** as part of the 42 curriculum.

The objective of this project is to recreate the core behavior of an IRC server using non-blocking sockets and the IRC protocol. Multiple clients can connect simultaneously, authenticate, join channels, exchange private messages, and interact using standard IRC commands.

The server uses a single `poll()` loop to handle all network events without using `fork()` or threads.

---

## Features

### Registration

- PASS
- NICK
- USER

### Messaging

- JOIN
- PART
- QUIT
- PRIVMSG (user ↔ user)
- PRIVMSG (channel)
- DCC file-transfer negotiation (SEND, RESUME, ACCEPT)

### Channel operators

- KICK
- INVITE
- TOPIC
- MODE

Supported channel modes:

- `+i` / `-i` : Invite-only channel
- `+t` / `-t` : Restrict topic changes to operators
- `+k` / `-k` : Set or remove a channel password
- `+o` / `-o` : Give or remove operator privileges
- `+l` / `-l` : Set or remove the user limit

---

## Technical Overview

- C++98
- TCP/IP sockets
- Non-blocking file descriptors
- Single `poll()` event loop
- Multiple simultaneous clients
- Packet buffering and command reconstruction
- Channel management
- Private messaging
- IRC numeric replies
- No external libraries

### Bonus

- Built-in IRC bot
- DCC file-transfer negotiation for HexChat

---

## Instructions

### Compilation

```bash
make
```

Other available rules:

```bash
make clean
make fclean
make re
```

### Execution

```bash
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 password
```

### Example using netcat

Open a terminal:

```bash
nc -C 127.0.0.1 6667
```

Register a client:

```text
PASS password
NICK alice
USER alice 0 * :Alice
```

Create or join a channel:

```text
JOIN #general
```

Leave a channel or disconnect from the server:

```text
PART #general :Leaving the channel
QUIT :Goodbye
```

Send a message:

```text
PRIVMSG #general :Hello everyone!
```

---

## Reference Client

The project was tested using:

- HexChat

---

## Project Structure

```
inc/
    Channel.hpp
    Client.hpp
    CommandHandler.hpp
    Macro.hpp
    Parser.hpp
    Server.hpp
    Utils.hpp

src/
    Channel.cpp
    ChannelMembers.cpp
    ChannelModes.cpp

    CommandDispatcher.cpp

    channelcommands/
        RegistrationCheck.cpp
        JoinCommand.cpp
        PartCommand.cpp
        QuitCommand.cpp
        PrivmsgCommand.cpp
        TopicCommand.cpp
        InviteCommand.cpp
        KickCommand.cpp
        ModeCommand.cpp
        BotCommand.cpp
        FileTransferHandle.cpp

    Client.cpp
    CommandHandler.cpp
    Parser.cpp
    Server.cpp
    Server_op.cpp
    Utils.cpp
    main.cpp
```

---

## Resources

- RFC 1459 — Internet Relay Chat Protocol  
  https://datatracker.ietf.org/doc/html/rfc1459

- RFC 2812 — Internet Relay Chat: Client Protocol  
  https://datatracker.ietf.org/doc/html/rfc2812

- Beej's Guide to Network Programming  
  https://beej.us/guide/bgnet/

- Linux manual pages:
  - socket
  - bind
  - listen
  - accept
  - recv
  - send
  - poll
  - fcntl

---

## AI Usage

Artificial intelligence tools were used as a learning and review aid during the development of this project.

AI assistance was mainly used for:

- understanding C++98 concepts;
- studying the IRC protocol and numeric replies;
- discussing possible implementations of channel commands and modes;
- reviewing code organization and refactoring;
- suggesting edge-case tests and improving documentation.

Every suggestion was reviewed, adapted, compiled, and tested before being integrated into the project.

---

## Authors

- **mohabid**
- **sihfadil**
