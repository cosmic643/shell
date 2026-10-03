#include <cstdio>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>


struct ParsedCommand{
    std::string output_file;
    std::string input_file;
    std::vector<std::string> command_tokens;
};


std::vector<std::string> tokenize(const std::string& cmd){
    std::vector<std::string> tokens;
    std::string temp;

    for (char c:cmd){
        if (c == ' '){
            if (!temp.empty()){
                tokens.push_back(temp);
                temp.clear();
            }
        }
        else{
            temp += c;
        }
    }
    if (!temp.empty()){
        tokens.push_back(temp);
    }
    return tokens;
}


ParsedCommand parse_redirection(const std::vector<std::string> tokens){
    ParsedCommand result;
    for (size_t i = 0;i < tokens.size();i++){
        if (tokens[i] == "<"){
            if (i+1 < tokens.size()){
                result.input_file = tokens[i+1];
                i++;
            }
            continue;
        }
        else if (tokens[i] == ">"){
            if (i + 1 < tokens.size()){
                result.output_file = tokens[i+1];
                i++;
            }
            continue;
        }
        result.command_tokens.push_back(tokens[i]);
    }
    return result;
}

bool redirect_output(const std::string& output_file) {
    int fd = open(
        output_file.c_str(),
        O_WRONLY | O_CREAT | O_TRUNC,
        0644
    );

    if (fd < 0) {
        perror("open");
        return false;
    }

    if (dup2(fd, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}

bool redirect_input(const std::string& input_file) {
    int fd = open(input_file.c_str(), O_RDONLY);

    if (fd < 0) {
        perror("open");
        return false;
    }

    if (dup2(fd, STDIN_FILENO) < 0) {
        perror("dup2");
        close(fd);
        return false;
    }

    close(fd);
    return true;
}


int main(){
    while(true){
        std::string cmd;
        std::cout<<getpid()<<" "<<"myshell> ";
        if (!std::getline(std::cin, cmd)) {
            break;
        }
        std::vector<char*> arguments;
    
        std::vector<std::string> tokens = tokenize(cmd);
        std::vector<std::string> command_tokens;

        size_t pipe_index = tokens.size();
        for (size_t i = 0;i < tokens.size();i++){
            if (tokens[i] == "|"){
                pipe_index = i;
                break;
            }
        }

        if (tokens.empty()){
                continue;
            }
            if (tokens[0] == "exit"){
                break;
            }
            if (tokens[0] == "cd"){
                const char* path;
                if (tokens.size() > 1){
                    path = tokens[1].c_str();
                }
                else{
                    path = getenv("HOME");
                }
        
                if (chdir(path) != 0){
                    std::cerr<<"Command Failed";
                }
                continue;
            }


        std::string output_file, input_file;
        ParsedCommand res = parse_redirection(tokens);
        output_file = res.output_file;
        input_file = res.input_file;
        command_tokens = res.command_tokens;
        
        if (command_tokens.empty()) {
            std::cerr << "Invalid command\n";
            continue;
        }
        
        for (auto& token: command_tokens){
            arguments.push_back(token.data());
        }
        arguments.push_back(nullptr);

        if (pipe_index == tokens.size())
        {
            
            
            pid_t pid = fork();
            if (pid < 0){
                std::cerr<<"Error in creating child process";   
            }
            else if(pid == 0){
                if(!output_file.empty() && !redirect_output(output_file)){
                    _exit(1);
                }
    
                if (!input_file.empty() && !redirect_input(input_file)){
                    _exit(1);
                }
                execvp(arguments[0],arguments.data());
                perror("execvp");
                _exit(1);
            }
            else{
                waitpid(pid, nullptr, 0);
            }
            continue;
        }
        if ( pipe_index == 0 || pipe_index + 1 >= tokens.size()) {
            std::cerr << "Invalid pipe\n";
            continue;
        }
        std::vector<std::string> left_tokens(tokens.begin(), tokens.begin()+pipe_index);
        std::vector<std::string> right_tokens(tokens.begin()+pipe_index+1, tokens.end());
        std::vector<std::string> left_command, right_command;
        std::string left_input, left_output, right_input, right_output;

        ParsedCommand left_res = parse_redirection(left_tokens);
        ParsedCommand right_res = parse_redirection(right_tokens);

        left_command = left_res.command_tokens;
        left_input = left_res.input_file;
        left_output = left_res.output_file;

        right_command = right_res.command_tokens;
        right_input = right_res.input_file;
        right_output = right_res.output_file;
        
        if (left_command.empty() || right_command.empty()){
            std::cerr<<"invalid command\n";
            continue;
        }
        std::vector<char*> left_args;
        for (auto& token : left_command) {
            left_args.push_back(token.data());
        }

        left_args.push_back(nullptr);


        std::vector<char*> right_args;

        for (auto& token : right_command) {
            right_args.push_back(token.data());
        }

        right_args.push_back(nullptr);

        int pipefd[2];
        
        if (pipe(pipefd) < 0) {
            perror("pipe");
            continue;
        }

        pid_t pid1 = fork();
        
        if (pid1 < 0) {
            perror("fork");

            close(pipefd[0]);
            close(pipefd[1]);

            continue;
        }
        
        if (pid1 == 0) {
            if (!left_input.empty() && !redirect_input(left_input)) {
                _exit(1);
            }
            if (!left_output.empty()) {
                if (!redirect_output(left_output)) {
                    _exit(1);
                }
            }
            else {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }


            close(pipefd[0]);
            close(pipefd[1]);

            execvp(
                left_args[0],
                left_args.data()
            );

            perror("execvp");
            _exit(1);
        }

        // ============================================================
        // CHILD 2 — RIGHT COMMAND
        // ============================================================

        pid_t pid2 = fork();

        if (pid2 < 0) {

            perror("fork");

            close(pipefd[0]);
            close(pipefd[1]);

            waitpid(pid1, nullptr, 0);

            continue;
        }

        if (pid2 == 0) {

            // ----------------------------
            // RIGHT INPUT
            // ----------------------------

            if (!right_input.empty()) {
                if(!redirect_input(right_input))
                    _exit(1);
            }
            else{
                if (dup2(pipefd[0], STDIN_FILENO) < 0) {
                    perror("dup2");
                    _exit(1);
                }
            }

            // ----------------------------
            // RIGHT OUTPUT >
            // ----------------------------

            if (!right_output.empty() && !redirect_output(right_output)) {
                _exit(1);
            }

            close(pipefd[0]);
            close(pipefd[1]);

            execvp(
                right_args[0],
                right_args.data()
            );

            perror("execvp");
            _exit(1);
        }

        // ============================================================
        // PARENT
        // ============================================================

        // Parent doesn't use the pipe.
        close(pipefd[0]);
        close(pipefd[1]);

        waitpid(pid1, nullptr, 0);
        waitpid(pid2, nullptr, 0);
    }
    return 0;
}