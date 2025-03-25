#include "client.hpp"

Client::Client() {}

Client::Client(pollfd pfd, std::string ip) : pfd(pfd), _ip(ip) {}

Client::~Client() {}

