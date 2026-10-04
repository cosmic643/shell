#include "redirection.hpp"

#include <cstdio>
#include <fcntl.h>
#include <unistd.h>


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

    int fd = open(
        input_file.c_str(),
        O_RDONLY
    );

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