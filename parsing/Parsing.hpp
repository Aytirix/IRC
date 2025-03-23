
#ifndef PARSING_HPP
#define PARSING_HPP

#include <list>
#include <string>
#include <map>
#include <unistd.h>
#include "../client/client.hpp"
#include "../server/server.hpp"
#include "../server/responses.hpp"
#include "../server/hasher.hpp"
#include "../log/log.hpp"

class Server;

class Parsing
{
private:
	Server &server;

public:
	Parsing(Server &server);
	~Parsing();

	void	clean_buffer(std::string &buffer);
	bool	init_parsing(Client &client, std::string &buffer);
};

#endif