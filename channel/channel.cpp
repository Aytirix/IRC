#include "channel.hpp"
#include <algorithm>

Channel::Channel(Server &server, std::string &name, Client &client) : _server(server), _name(name)
{
	_operators.push_back(client.getSocketFd());
	addClient(client);
}

Channel::~Channel() {}

void Channel::addClient(Client client)
{

	_clients[client.getSocketFd()] = client;
	this->broadcastMessage(client, USER_JOIN_CHANNEL(client.getUniqueName(), _name));
}

void Channel::addOperator(Client &client)
{
	_operators.push_back(client.getSocketFd());
}

void Channel::removeOperator(Client &client)
{
	_operators.remove(client.getSocketFd());
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
	{
		if (std::find(_operators.begin(), _operators.end(), it->first) != _operators.end())
			clients += "@" + it->second.getNickname() + " ";
		else
			clients += it->second.getNickname() + " ";
	}
	clients.substr(0, clients.size() - 1);
	return clients;
}

void Channel::broadcastMessage(std::string message)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		_server.send_data(it->first, message, false, true);
}

void Channel::broadcastMessage(Client client, std::string message)
{
	for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->first != client.getSocketFd())
			_server.send_data(it->first, message, false, true);
	}
}