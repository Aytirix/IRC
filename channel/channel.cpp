#include "channel.hpp"

Channel::Channel(Server &server, std::string &name, Client &client) : _server(server), _name(name) {}

Channel::~Channel() {}