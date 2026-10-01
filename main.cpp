#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <vector>

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

int main(){
    while(true){
        std::string cmd;
        std::cout<<getpid()<<" "<<"myshell> ";
        std::getline(std::cin, cmd);
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
        if (pipe_index == tokens.size()) {
            std::cerr << "No pipe found\n";
            return 1;
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
        for (size_t i= 0; i < tokens.size(); i++){
            if (tokens[i] == ">"){
                if (i+1 < tokens.size()) {
                    output_file = tokens[i+1];
                    i++;
                }
                continue;
            }
            else if (tokens[i] == "<"){
                if (i+1 < tokens.size()) {
                    input_file = tokens[i+1];
                    i++;
                }
                continue;
            }
            command_tokens.push_back(tokens[i]);
        }

        
        
        if (command_tokens.empty()) {
            std::cerr << "Invalid command\n";
            continue;
        }
        
        if (!output_file.empty() == false &&
            std::find(tokens.begin(), tokens.end(), ">") != tokens.end()) {
            std::cerr << "Missing output file\n";
            continue;
        }
        
        for (auto& token: command_tokens){
            arguments.push_back(token.data());
        }
        arguments.push_back(nullptr);

        if (pipe_index == tokens.size())
        {
            if(!output_file.empty()){
                int fd = open(output_file.c_str(),
                O_WRONLY | O_CREAT | O_TRUNC,
                0644);
                if (fd < 0){
                    perror("open");
                    _exit(1);
                }
                if (dup2(fd, STDOUT_FILENO) < 0) {
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd);
            }

            if (!input_file.empty()){
                int fd = open(input_file.c_str(), O_RDONLY);
                if (fd < 0){
                    perror("open");
                    _exit(1);
                }
                if (dup2(fd, STDIN_FILENO) < 0){
                    perror("dup2");
                    close(fd);
                    _exit(1);
                }
                close(fd); 
            }
            pid_t pid = fork();
            if (pid < 0){
                std::cerr<<"Error in creating child process";   
            }
            else if(pid == 0){
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
        std::vector<std::string> left_command;
        std::string left_input, left_output;
        for (size_t i = 0; i < left_tokens.size(); i++) {
            if (left_tokens[i] == "<") {

                if (i + 1 < left_tokens.size()) {
                    left_input = left_tokens[i + 1];
                    i++;
                }

                continue;
            }

            if (left_tokens[i] == ">") {

                if (i + 1 < left_tokens.size()) {
                    left_output = left_tokens[i + 1];
                    i++;
                }

                continue;
            }

            left_command.push_back(left_tokens[i]);
        }

        std::vector<std::string> right_command;
        std::string right_input, right_output;
        for (size_t i = 0;i < right_tokens.size();i++){
            if (right_tokens[i] == "<") {
                if (i + 1 < right_tokens.size()) {
                    right_input = right_tokens[i + 1];
                    i++;
                }
                continue;
            }
            if (right_tokens[i] == ">") {
                if (i + 1 < right_tokens.size()) {
                    right_output = right_tokens[i + 1];
                    i++;
                }
                continue;
            }
            right_command.push_back(right_tokens[i]);
        }
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
            if (!left_input.empty()) {
                int fd = open(
                    left_input.c_str(),
                    O_RDONLY
                );

                if (fd < 0) {
                    perror("open");
                    _exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);
            }
            if (!left_output.empty()) {

                // User explicitly asked for >
                int fd = open(
                    left_output.c_str(),
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644
                );

                if (fd < 0) {
                    perror("open");
                    _exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);
            }

            else {
                dup2(
                    pipefd[1],
                    STDOUT_FILENO
                );
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

                // User explicitly used <
                int fd = open(
                    right_input.c_str(),
                    O_RDONLY
                );

                if (fd < 0) {
                    perror("open");
                    _exit(1);
                }

                dup2(fd, STDIN_FILENO);
                close(fd);
            }

            else {

                // No <
                // therefore stdin comes from pipe

                dup2(
                    pipefd[0],
                    STDIN_FILENO
                );
            }

            // ----------------------------
            // RIGHT OUTPUT >
            // ----------------------------

            if (!right_output.empty()) {

                int fd = open(
                    right_output.c_str(),
                    O_WRONLY | O_CREAT | O_TRUNC,
                    0644
                );

                if (fd < 0) {
                    perror("open");
                    _exit(1);
                }

                dup2(fd, STDOUT_FILENO);
                close(fd);
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