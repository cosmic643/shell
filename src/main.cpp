#include "parser.hpp"
#include "executor.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>


int main() {

    while (true) {

        std::string cmd;

        std::cout << getpid() << " myshell> ";

        if (!std::getline(std::cin, cmd)) {
            break;
        }


        std::vector<std::string> tokens = tokenize(cmd);
        
        if (tokens.empty()) {
            continue;
        }


        // BUILTINS

        if (tokens[0] == "exit") {
            break;
        }


        if (tokens[0] == "cd") {

            const char* path;

            if (tokens.size() > 1) {
                path = tokens[1].c_str();
            }
            else {
                path = getenv("HOME");
            }

            if (path == nullptr) {
                std::cerr << "HOME not set\n";
                continue;
            }

            if (chdir(path) != 0) {
                perror("cd");
            }

            continue;
        }


        // FIND PIPE

        size_t pipe_index = tokens.size();

        for (size_t i = 0; i < tokens.size(); i++) {

            if (tokens[i] == "|") {
                pipe_index = i;
                break;
            }
        }


        // NORMAL COMMAND

        if (pipe_index == tokens.size()) {

            ParsedCommand command =
                parse_redirection(tokens);

            if (command.command_tokens.empty()) {
                std::cerr << "Invalid command\n";
                continue;
            }

            execute_command(command);

            continue;
        }


        // PIPE VALIDATION

        if (
            pipe_index == 0 ||
            pipe_index + 1 >= tokens.size()
        ) {
            std::cerr << "Invalid pipe\n";
            continue;
        }


        // SPLIT PIPE

        std::vector<std::string> left_tokens(
            tokens.begin(),
            tokens.begin() + pipe_index
        );

        std::vector<std::string> right_tokens(
            tokens.begin() + pipe_index + 1,
            tokens.end()
        );


        ParsedCommand left =
            parse_redirection(left_tokens);

        ParsedCommand right =
            parse_redirection(right_tokens);


        if (
            left.command_tokens.empty() ||
            right.command_tokens.empty()
        ) {
            std::cerr << "Invalid command\n";
            continue;
        }


        execute_pipeline(left, right);
    }

    return 0;
}