### This project has been created as part of the 42 curriculum by mohabid and sihfadil

## Description:

-This project has been made by the 42 pedagogical teams to create an IRC server, by establishing a TCP
socket programming connection via a port and a password, once a client connected with the port number,
he or she enters the password of the server and they should be able to having the following commands
as for them to exist in your server to chat and do things you know:

# 1st: Registration process:

-PASS first command that must be executed, why? to join the server ofc.
-NICK to create a nickname or update the old one.
-USER to identify user's usernam and real name.

# 2nd: Client's playground:

-JOIN to create a new channel if it doesn't exist or join an existing channel.
-PRIVMSG to send private messages to some other client or to all the members in a channel.
-PART to leave a channel.
-QUIT to disconnect from  a server.

# 3rd: IRCop or the channel operator:

what is a channel operator? a channel operator is nothing but a privilieged client that enforce network
rules and manage its infrastructure. an IRCop should be able to execute the following commands:
- **KICK**  Remove a client from a channel.
- **INVITE**  Invite a client to a channel.
- **TOPIC**  View or change the channel topic.
- **MODE**  Manage channel modes:
  - `i` : invite-only channel.
  - `t` : only operators can change the topic.
  - `k` : set or remove a channel password.
  - `o` : give or remove operator privileges.
  - `l` : set or remove the user limit.

  