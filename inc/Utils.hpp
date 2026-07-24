#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <cerrno>
#include <cstdlib>
bool isValidNickname(const std::string& nick);
std::string makeReply(const std::string& code,
                      const std::string& target,
                      const std::string& msg);
std::string clientPrefix(const std::string& nickname,
                         const std::string& username);
bool ircCaseEqual(const std::string& left, const std::string& right);
bool isValidPort(const char* value, int& port);
#endif
