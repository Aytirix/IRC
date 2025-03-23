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
    std::string _username;
	std::string _ip;
	std::string _realname;
	std::string buffer;
public:
	Client();
    Client(pollfd socket_fd);
    ~Client();

    // Getters
    int getSocketFd() const { return this->pfd.fd; }
	std::string &getBuffer() { return this->buffer; }
	std::string getUniqueName() const { return this->_nickName + "!" + this->_username + "@" + this->_ip;}
	std::string getUserName() const { return this->_username; }
    pollfd getSocketPfd() const { return this->pfd; }
    std::string getNickname() const { return this->_nickName; }
	std::string getRealName() const { return this->_realname; }
	std::string getIp() const { return this->_ip; }

    // Setters
	void setSocketPfd(pollfd pfd) { this->pfd = pfd; }
    void setNickname(const std::string &username) { this->_nickName = username; }
	void setUserName(const std::string &username) { this->_username = username; }
	void setRealName(const std::string &realname) { this->_realname = realname; }
	void setIp(const std::string &ip) { this->_ip = ip; }
};

#endif
