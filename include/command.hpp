#pragma once

enum class RedirectionType {
    Input,
    Output
};

struct Redirection {
    RedirectionType type;
    std::string file;
};

struct ParsedCommand {
    std::vector<Redirection> redirections;
    std::vector<std::string> command_tokens;
    void clear(){
        *this = ParsedCommand();
    }
};