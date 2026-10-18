#include "parser.hpp"
#include "executor.hpp"

#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>


int main() {

    while (true) {

        while (waitpid(-1, nullptr, WNOHANG) > 0) {}

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

        ParseResult parse_result = parse_tokens(tokens);
        if (parse_result.valid){
            execute_pipeline(parse_result.commands, parse_result.background);
        }
        else {
            std::cerr << "Invalid command\n";
        }
    }
    return 0;
}