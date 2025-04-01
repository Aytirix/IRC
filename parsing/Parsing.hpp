
#ifndef PARSING_HPP
#define PARSING_HPP

#include <list>
#include <string>
#include <sstream>
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
	void	capabilities(Client &client, std::vector<std::string> &v_buffer);
	void	clean_buffer(std::string &buffer);
	bool	pass(Client &client, std::vector<std::string> &v_buffer);
	void	nick(Client &client, std::vector<std::string> &v_buffer);
	void	user(Client &client, std::vector<std::string> &v_buffer);
public:
	Parsing(Server &server);
	~Parsing();

	bool	init_parsing(Client &client, std::string &buffer);
};

#endif