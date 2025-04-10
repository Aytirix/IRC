#include "client.hpp"

Client::Client() {}

Client::Client(pollfd pfd, std::string ip) : pfd(pfd), _nickName(""), _username(""), _ip(ip), password_verified(false) {}

Client::~Client() {}
