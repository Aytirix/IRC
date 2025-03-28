
#ifndef PARSING_HPP
#define PARSING_HPP

#include <list>
#include <string>
#include <vector>
#include <unistd.h>

#include "../client/client.hpp"

class Server;
class Chatbot;

class Parsing
{
private:
	Server &server;

	void capability(Client &client, std::string &args);
	bool check_enough_params(Client &client, std::string &command, std::string &args);

	// CHANNEL
	void Who(Client &client, std::string &channel);
	void joinChannel(Client &client, std::string &channelName);
	void partChannel(Client &client, std::string &args);
	void PRIVMSG(Client &client, std::string &args);
	void CMD_KICK(Client &client, std::string &args);

	// CLIENT
	bool CMD_PASS(Client &client, std::string &password);
	bool CMD_USER(Client &client, std::string &username);
	void CMD_NICK(Client &client, std::string &nickname);


public:
	Parsing(Server &server);

	~Parsing();

	bool init_parsing(Client &client, std::string &buffer);
	std::string RemoveHiddenChar(std::string &str);
	std::vector<std::string> split(const std::string &str, char delim);
	std::string toLower(std::string str);
};

#endif