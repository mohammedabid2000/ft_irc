#include "../inc/Utils.hpp"

static char ircLower(char c)
{
    if (c >= 'A' && c <= 'Z')
        return static_cast<char>(c - 'A' + 'a');
    if (c == '[')
        return '{';
    if (c == ']')
        return '}';
    if (c == '\\')
        return '|';
    if (c == '^')
        return '~';
    return c;
}

bool ircCaseEqual(const std::string& left, const std::string& right)
{
    if (left.size() != right.size())
        return false;
    for (size_t i = 0; i < left.size(); ++i)
    {
        if (ircLower(left[i]) != ircLower(right[i]))
            return false;
    }
    return true;
}

bool isValidNickname(const std::string& nick)
{
    if (nick.empty() || nick.length() > 9)
        return false;

    const std::string allowedFirst =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz[]\\`_^{|}";
    const std::string allowedRest =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-[]\\`_^{|}";

    if (allowedFirst.find(nick[0]) == std::string::npos)
        return false;

    for (size_t i = 1; i < nick.size(); ++i)
    {
        if (allowedRest.find(nick[i]) == std::string::npos)
            return false;
    }
    return true;
}

std::string makeReply(const std::string& code,
                      const std::string& target,
                      const std::string& msg)
{
    return ":" + std::string("ircserv") + " " + code + " " + target
        + " :" + msg + "\r\n";
}

std::string clientPrefix(const std::string& nickname,
                         const std::string& username)
{
    return ":" + nickname + "!" + username + "@localhost";
}

bool isValidPort(const char* value, int& port)
{
    if (!value || !value[0])
        return false;
    unsigned long number = 0;
    for (size_t i = 0; value[i]; ++i)
    {
        if (value[i] < '0' || value[i] > '9')
            return false;
        number = number * 10 + static_cast<unsigned long>(value[i] - '0');
        if (number > 65535)
            return false;
    }
    if (number == 0)
        return false;
    port = static_cast<int>(number);
    return true;
}
