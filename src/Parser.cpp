#include "../inc/Parser.hpp"
#include <cctype>

ParsedCommand Parser::parse(const std::string& line)
{
    ParsedCommand cmd;
    size_t pos = 0;

    while (pos < line.size() && line[pos] == ' ')
        ++pos;

    size_t space = line.find(' ', pos);
    if (space == std::string::npos)
    {
        cmd.command = line.substr(pos);
    }
    else
    {
        cmd.command = line.substr(pos, space - pos);
        pos = space + 1;

        while (pos < line.length())
        {
            while (pos < line.length() && line[pos] == ' ')
                ++pos;
            if (pos >= line.length())
                break;
            if (line[pos] == ':')
            {
                cmd.params.push_back(line.substr(pos + 1));
                break;
            }
            space = line.find(' ', pos);
            if (space == std::string::npos)
            {
                cmd.params.push_back(line.substr(pos));
                break;
            }
            cmd.params.push_back(line.substr(pos, space - pos));
            pos = space + 1;
        }
    }

    for (size_t i = 0; i < cmd.command.size(); ++i)
    {
        cmd.command[i] = static_cast<char>(
            std::toupper(static_cast<unsigned char>(cmd.command[i])));
    }
    return cmd;
}
