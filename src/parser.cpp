#include "parser.hpp"
#include "command.hpp"
#include <cstddef>
#include <vector>


bool is_operator(const std::string& token){
    if (token == "|" || token == "<" || token == ">" || token == "&") return true;
    return false;
}


std::vector<std::string> tokenize(const std::string& cmd) {
    std::vector<std::string> tokens;
    std::string temp;

    for (char c : cmd) {
        if (c == ' ') {
            if (!temp.empty()) {
                tokens.push_back(temp);
                temp.clear();
            }
        }
        else if (c == '<' || c == '>' || c == '|' || c == '&'){
            if (!temp.empty()) {
                tokens.push_back(temp);
                temp.clear();
            }
            tokens.emplace_back(1,c);
        }
        else {
            temp += c;
        }
    }

    if (!temp.empty()) {
        tokens.push_back(temp);
    }

    return tokens;
}


ParseResult parse_tokens(const std::vector<std::string> &tokens){
    std::vector<ParsedCommand> result;
    ParsedCommand parsed_cmd;
    bool background = false;
    size_t n = tokens.size();
    
    for (size_t i = 0;i < n;i++){
        if (tokens[i] == "|") {
            if (parsed_cmd.command_tokens.empty()) {
                return ParseResult{{}, false, false};
            }
            result.push_back(parsed_cmd);
            parsed_cmd.clear();
        }
        
        else if (tokens[i] == "<") {
            
            if (i + 1 >= n || is_operator(tokens[i + 1])){
                return ParseResult{{}, , false};
            }
            parsed_cmd.redirections.push_back({RedirectionType::Input, tokens[++i]});
        }

        else if (tokens[i] == ">") {
            if (i + 1 >= n || is_operator(tokens[i + 1])) {
                return ParseResult{{},false, false};
            }
            parsed_cmd.redirections.push_back({RedirectionType::Output, tokens[++i]});
        }
        else if (tokens[i] == "&") {
            if (i != n-1){
                return ParseResult{{}, false, false}
            }
            background = true;
        }
        else {
            parsed_cmd.command_tokens.push_back(tokens[i]);
        }
    }
    
    if (parsed_cmd.command_tokens.empty()) {
        return ParseResult{{}, background, false};
    }
    
    result.push_back(parsed_cmd);
    
    return ParseResult{result, background, true};
}






// ParsedCommand parse_redirection(
//     const std::vector<std::string>& tokens
// ) {
//     ParsedCommand result;

//     for (size_t i = 0; i < tokens.size(); i++) {

//         if (tokens[i] == "<") {
//             if (i + 1 < tokens.size()) {
//                 result.input_file = tokens[i + 1];
//                 i++;
//             }
//             continue;
//         }

//         if (tokens[i] == ">") {
//             if (i + 1 < tokens.size()) {
//                 result.output_file = tokens[i + 1];
//                 i++;
//             }
//             continue;
//         }

//         result.command_tokens.push_back(tokens[i]);
//     }

//     return result;
// }