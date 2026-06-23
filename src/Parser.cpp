#include "../inc/Parser.hpp"

ParsedCommand Parser::parse(const std::string &line)
{
    ParsedCommand cmd;
    size_t pos = 0;
    size_t space;
    space = line.find(' ');
    if (space == std::string::npos)
    {
        cmd.command = line;
        return cmd;
    }
    cmd.command = line.substr(0, space);
    pos = space + 1;
    while (pos < line.length())
    {
        while (pos < line.length() && line[pos] == ' ')
            pos++;
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
    return cmd;
}