#ifndef PARSER_HPP
#define PARSER_HPP
#include <string>
#include <vector>
#include <sstream>
class ParsedCommand
{
    public:
        std::string command;
        std::vector<std::string> params;
};
class Parser
{
    public:
        ParsedCommand parse(const std::string& line);
};
#endif