#ifndef MACRO_HPP
#define MACRO_HPP

#define SERVER_NAME "ircserv"
#define SERVER_VERSION "1.0"

#define RPL_WELCOME              "001"
#define RPL_NOTOPIC              "331"
#define RPL_NAMREPLY             "353"
#define RPL_ENDOFNAMES           "366"

#define ERR_NOSUCHNICK           "401"
#define ERR_NOSUCHCHANNEL        "403"
#define ERR_CANNOTSENDTOCHAN     "404"
#define ERR_NORECIPIENT          "411"
#define ERR_NOTEXTTOSEND         "412"
#define ERR_UNKNOWNCOMMAND       "421"
#define ERR_NONICKNAMEGIVEN      "431"
#define ERR_ERRONEUSNICKNAME     "432"
#define ERR_NICKNAMEINUSE        "433"
#define ERR_NOTREGISTERED        "451"
#define ERR_NEEDMOREPARAMS       "461"
#define ERR_ALREADYREGISTERED    "462"
#define ERR_PASSWDMISMATCH       "464"

#define ERR_NEEDMOREPARAMS_PASS  ERR_NEEDMOREPARAMS
#define ERR_NEEDMOREPARAMS_USER  ERR_NEEDMOREPARAMS

#endif
