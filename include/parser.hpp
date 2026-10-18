#pragma once

#include <string>
#include <vector>
#include "command.hpp"


struct ParseResult {
    std::vector<ParsedCommand> commands;
    book background;
    bool valid;
};


std::vector<std::string> tokenize(const std::string& cmd);

ParsedCommand parse_redirection(
    const std::vector<std::string>& tokens
);

std::vector<ParsedCommand> parse_tokens(
    const std::vector<std::string>& tokens
);