
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

	void CMD_CAP(Client *client, std::string &args);
	bool check_enough_params(Client *client, std::string &command, std::string &args);

	// CHANNEL
	void CMD_WHO(Client *client, std::string &channel);
	void CMD_JOIN(Client *client, std::string &channelName);
	void CMD_PART(Client *client, std::string &args);
	void CMD_PRIVMSG(Client *client, std::string &args);
	void CMD_KICK(Client *client, std::string &args);
	void CMD_TOPIC(Client *client, std::string &args);
	void CMD_INVITE(Client *client, std::string &args);
	void CMD_LIST(Client *client, std::string &channel);
	void CMD_MODE(Client *client, std::string &args);

	// CLIENT
	bool CMD_PASS(Client *client, std::string &password);
	bool CMD_USER(Client *client, std::string &username);
	void CMD_NICK(Client *client, std::string &nickname);


public:
	Parsing(Server &server);

	~Parsing();

	bool init_parsing(Client *client, std::string &buffer);
	std::string RemoveHiddenChar(std::string &str);
	std::vector<std::string> split(const std::string &str, char delim);
	std::string toLower(std::string str);
};

#endif