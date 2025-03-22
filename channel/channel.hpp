#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <poll.h>
#include <list>
#include <string>
#include <map>
#include <unistd.h>
#include "../client/client.hpp"
#include "../server/server.hpp"

class Client;
class Server;

class Channel
{
private:
	Server &_server;
	std::string &_name;
	std::map<int, Client> _clients;
	std::vector<int> _operators;
	int limit;
	bool _mode_topic;

public:

	Channel(Server &server,std::string &name, Client &client);
    ~Channel();

	// Methods

    // Getters

	// Setters
};

#endif
