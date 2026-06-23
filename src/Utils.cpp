#include "../inc/Utils.hpp"
bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;
    char first = nick[0];
    std::string allowedFirst = "ABCDEFGHIJKLMNOPQRSTUVWXYZ""abcdefghijklmnopqrstuvwxyz""[]\\`_^{|}";
    if (allowedFirst.find(first) == std::string::npos)
        return false;
    for (size_t i = 1; i < nick.size(); i++)
    {
        char c = nick[i];
        std::string allowed = "ABCDEFGHIJKLMNOPQRSTUVWXYZ""abcdefghijklmnopqrstuvwxyz""0123456789""-[]\\`_^{|}";
        if (allowed.find(c) == std::string::npos)
            return false;
    }

    return true;
}
std::string makeReply(const std::string& code, const std::string& target, const std::string& msg)
{
    return ":ircserv "+ code +" " + target +" :" + msg +"\r\n";
}