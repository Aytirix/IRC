#include "client.hpp"

Client::Client() {}

Client::Client(pollfd pfd)
{
	this->pfd = pfd;
}

Client::~Client() {}
