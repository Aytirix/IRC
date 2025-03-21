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
	int _operator;
	int limit;
	bool _mode_topic;

public:

	Channel(Server &server,std::string &name, Client &client);
    ~Channel();

	// Methods
	void addClient(Client client);
	void removeClient(Client client);
	void addMessage(std::string message);
	void sendMessage(Client client, std::string message);
	void broadcastMessage(std::string message);
	void broadcastMessage(Client client, std::string message);
	void changeOperator(Client &client);

    // Getters
	std::list<std::string> getMessages();
	std::map<int, pollfd> getClients();	
    // Setters
};

#endif
