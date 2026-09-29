#include <cstdlib>
#include <iostream>
#include <string>
#include <unistd.h>
#include <filesystem>
#include <vector>

std::vector<std::string> tokenize(std::string cmd){
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
     std::vector<char*> args;

     std::vector<std::string> tokens= tokenize(cmd);
     

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

     for (auto& token: tokens){
         args.push_back(token.data());
     }
     args.push_back(nullptr);
     pid_t pid = fork();
     if (pid < 0){
      std::cerr<<"Error in creating child process";   
     }
     else if(pid == 0){
         execvp(args[0],args.data());
         std::cerr<<"Execvp Failed";
         _exit(1);
     }
     else{
         waitpid(pid, nullptr, 0);
     }
    }
    return 0;
}