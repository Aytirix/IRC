#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <poll.h>
#include <list>
#include <string>
#include <map>
#include <unistd.h>
#include <algorithm>

#include "../client/client.hpp"

class Client;
class Server;

struct Client_channel
{
	Client _client;
	bool _connected;
	bool _operator;
};

class Channel
{
private:
	Server &_server;
	std::string _name;
	std::map<int, Client_channel> _clients;
	std::list<int> _operators;
	int limit; // A implementer -l
	bool _mode_topic; // A implementer -t
	std::string password; // A implenter -k
public:
	Channel(Server &server,std::string &name, Client &client);
    ~Channel();

	// Methods
	void addClient(Client client, bool _operator = false);
	bool disconnectClientChannel(Client &client);
	void broadcastMessage(std::string message);
	void broadcastMessage(Client &client, std::string message);
	void addOperator(Client_channel &client);
	void removeOperator(Client_channel &client);
	std::string getAllClientsString();
	void kickClient(Client &client, std::string &client_kick, std::string &message);

    // Getters
	Client_channel getClientByNickname(std::string &nickname);
	std::string getname() { return _name; };
    // Setters
};

#endif
