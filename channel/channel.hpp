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

struct Client_channel
{
	Client _client;
	bool is_connected; // pour le ban
	bool _is_operator;
};

class Channel
{
private:
	Server &_server;
	std::string &_name;
	std::map<int, Client> _clients;
	std::list<int> _operators;
	int limit; // A implementer -l
	bool _mode_topic; // A implementer -t
	std::string password; // A implenter -k

public:

	Channel(Server &server,std::string &name, Client &client);
    ~Channel();

	// Methods
	void addClient(Client client);
	bool removeClient(Client client);
	void broadcastMessage(std::string message);
	void broadcastMessage(Client client, std::string message);
	void addOperator(Client &client);
	void removeOperator(Client &client);
	std::string getAllClientsString();

    // Getters
	std::list<std::string> getMessages();
	std::map<int, pollfd> getClients();	
    // Setters
};

#endif
