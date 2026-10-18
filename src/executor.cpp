#include "executor.hpp"
#include "redirection.hpp"

#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <vector>
#include <array>

int execute_pipeline(std::vector<ParsedCommand>& commands, bool background){
    std::vector<std::array<int, 2>> pipes(commands.size() - 1);
    
    for (auto& p : pipes) {
        if (pipe(p.data()) < 0) {
            perror("pipe");
            return 1;
        }
    }
    std::vector<pid_t> pids;
    for (size_t i = 0; i < commands.size();i++){
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return 1;
        }
        
        if(pid > 0){
            if (i>0){
                if(dup2(pipes[i][1],STDIN_FILENO) < 0);
                {
                    perror("dup2");
                    _exit(1);
                }
            }
            if (i < n - 1) {
                if (dup2(
                    pipes[i][1],
                    STDOUT_FILENO
                ) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }
            for (const auto &r: commands[i].redirection){
                if (r.type == RedirectionType::Input){
                    if (!redirect_input(r.file)){
                        _exit(1);
                    }
                }
                else if (r.type == RedirecctionType::Output){
                    if (!redirect_output(r.file)){
                        _exit(1);
                    }
                }
            }

            for (auto& p : pipes) {
                close(p[0]);
                close(p[1]);
            }

            std::vector<char*> args;
           
            for (auto& token : commands[i].command_tokens) {
                args.push_back(token.data());
            }
            args.push_back(nullptr);

            execvp(args[0], args.data());
            perror("execvp");
            _exit(1);
        }
        pids.push_back(pid);
    }
    for (auto& p : pipes) {
        close(p[0]);
        close(p[1]);
    }
    
    
    // Wait for every command in the pipeline
    if (!background){
        for (pid_t pid : pids) {
            waitpid(pid, nullptr, 0);
        }
    }
    return 0;
}