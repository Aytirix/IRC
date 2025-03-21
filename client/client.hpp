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
    std::string _nickName;
	std::string buffer;
public:
	Client();
    Client(pollfd socket_fd);
    ~Client();

    // Getters
    pollfd getSocketPfd() const { return this->pfd; }
    int getSocketFd() const { return this->pfd.fd; }
    std::string getNickname() const { return this->_nickName; }
	std::string &getBuffer() { return this->buffer; }

    // Setters
    void setNickname(const std::string &username) { this->_nickName = username; }
	void setSocketPfd(pollfd pfd) { this->pfd = pfd; }
};

#endif
