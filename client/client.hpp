
#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <list>
#include <string>

class Client
{
private:
	int socket_fd;
	std::string username;

public:
	Client(int socket_fd, const std::string &username);

	~Client();

	// Getters
	int getSocketFd() const { return this->socket_fd; }
	std::string getUsername() const { return this->username; }

	// Setters
	void setUsername(const std::string &username);
	void setSocketFd(int socket_fd);
};

#endif