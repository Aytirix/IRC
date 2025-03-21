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

void Channel::removeClient(Client client)
{
	_clients.erase(client.getSocketFd());
}

void Channel::sendMessage(Client client, std::string message)
{
	(void)client;
	(void)message;
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

void Server::joinChannel(Client &client, std::string &channelName)
{
	std::map<std::string, Channel>::iterator it = channels_.find(channelName);
	if (it == channels_.end())
	{
		Channel channel(*this, channelName, client);
		channels_.insert(std::make_pair(channelName, channel));
		return;
	}
	it->second.addClient(client);
	this->send_data(client.getSocketFd(), ":"+ client.getNickname() +" JOIN "+ channelName, false, true);
	// this->send_data(client.getSocketFd(), ":thmouty1 JOIN " + channelName);
}