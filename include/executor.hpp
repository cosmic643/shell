#pragma once

#include "command.hpp"
#include "parser.hpp"

void execute_command(ParsedCommand& command);

int execute_pipeline(std::vector<ParsedCommand>& commands);