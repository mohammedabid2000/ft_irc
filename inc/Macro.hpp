#ifndef MACRO_HPP
#define MACRO_HPP

// =====================
// Server identity
// =====================
#define SERVER_NAME "ircserv"

// =====================
// PASS errors
// =====================
#define ERR_NEEDMOREPARAMS_PASS  "461"
#define ERR_ALREADYREGISTERED     "462"
#define ERR_PASSWDMISMATCH       "464"

// =====================
// NICK errors
// =====================
#define ERR_NONICKNAMEGIVEN      "431"
#define ERR_ERRONEUSNICKNAME     "432"
#define ERR_NICKNAMEINUSE        "433"
#define ERR_NICKCOLLISION        "436"

// =====================
// USER errors
// =====================
#define ERR_NEEDMOREPARAMS_USER  "461"

// (same 461 reused, but you may alias it)
#define ERR_ALREADYREGISTERED_USER "462"

#endif