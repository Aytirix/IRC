#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <poll.h>
#include <list>
#include <string>
#include <map>
#include <unistd.h>

class Client
{
private:
    pollfd pfd;
    std::string username;
	std::string buffer;
public:
    Client(pollfd socket_fd);
    ~Client();

    // Getters
    pollfd getSocketPfd() const { return this->pfd; }
    int getSocketFd() const { return this->pfd.fd; }
    std::string getUsername() const { return this->username; }
	std::string &getBuffer() { return this->buffer; }

    // Setters
    void setUsername(const std::string &username);
};

#endif
