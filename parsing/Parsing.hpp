
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
	bool check_enough_params(Client &client, std::string &buffer);
	bool IsRegistered(Client &client, std::string &buffer);

	// CHANNEL
	void Who(Client &client, std::string &buffer);
	void joinChannel(Client &client, std::string &channelName);
	void partChannel(Client &client, std::string &channelName, std::string &message);
	void PRIVMSG(Client &client, std::string &buffer);

public:
	Parsing(Server &server);

	~Parsing();

	bool init_parsing(Client &client, std::string &buffer);
	std::string RemoveHiddenChar(std::string &str);
	std::vector<std::string> split(const std::string &str, char delim);
};

#endif