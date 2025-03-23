#include "channel.hpp"

Channel::Channel(Server &server, std::string &name, Client &client) : _server(server), _name(name), _operator(client.getSocketFd())
{
	addClient(client);
}

Channel::~Channel() {}

void Channel::addClient(Client client)
{
	
	_clients[client.getSocketFd()] = client;
}

void Channel::changeOperator(Client &client)
{
	_operator = client.getSocketFd();
}

bool Channel::removeClient(Client client)
{
	if (_clients.find(client.getSocketFd()) == _clients.end())
		return false;
	_clients.erase(client.getSocketFd());
	return true;
}

void Channel::sendMessage(Client client, std::string message)
{
	(void)client;
	(void)message;
}

std::string Channel::getAllClientsString()
{
	std::string clients;
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		clients += it->second.getNickname() + " ";
	clients.substr(0, clients.size() - 1);
	return clients;
}

void Channel::broadcastMessage(std::string message)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		_server.send_data(it->first, message, true, true);
}

void Channel::broadcastMessage(Client client, std::string message)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != client.getSocketFd())
			_server.send_data(it->first, message, true, true);
	}
}