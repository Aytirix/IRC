#include "client.hpp"
#include <unistd.h>

Client::Client(int socket_fd, const std::string& username)
{
	this->socket_fd = socket_fd;
	this->username = username;
}

Client::~Client()
{
	close(this->socket_fd);
}

void Client::setUsername(const std::string& username)
{
	this->username = username;
}

void Client::setSocketFd(int socket_fd)
{
	this->socket_fd = socket_fd;
}