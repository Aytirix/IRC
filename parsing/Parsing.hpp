
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

	bool capability(Client &client, std::string &buffer);
	bool InitializeUser(Client &client, std::string &buffer);
public:
	Parsing(Server &server);

	~Parsing();

	bool init_parsing(Client &client, std::string &buffer);
	std::string RemoveHiddenChar(std::string &str);
	std::vector<std::string> split(const std::string &str, char delim);
};

#endif