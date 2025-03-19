#include "client.hpp"

Client::Client(pollfd pfd)
{
	this->pfd = pfd;
}

Client::~Client() {}

void Client::setUsername(const std::string &username)
{
	this->username = username;
}
