#include "server.hpp"
#include "../client/client.hpp"
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>

Server::Server(int port, const std::string &password)
{
	this->port = port;
	this->password = password;
	this->socket_fd = -1;
}

Server::~Server()
{
	for (std::map<int, Client *>::iterator it = this->clients.begin(); it != this->clients.end(); ++it)
	{
		delete it->second;
	}
	close(this->socket_fd);
}

void Server::start()
{
	this->socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->socket_fd == -1)
	{
		std::cerr << "Error creating socket." << std::endl;
		return;
	}

	sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(this->port);

	if (bind(this->socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		std::cerr << "Error binding socket." << std::endl;
		return;
	}

	if (listen(this->socket_fd, 10) == -1)
	{
		std::cerr << "Error listening on socket." << std::endl;
		return;
	}

	std::cout << "Server started on port " << this->port << std::endl;
}

void Server::loop()
{
	while (this->socket_fd != -1)
	{
		this->acceptClient();
		this->handleConnections();
	}
}

void Server::acceptClient()
{
	sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_socket = accept(this->socket_fd, (struct sockaddr *)&client_addr, &client_len);
	if (client_socket == -1)
	{
		std::cerr << "Error accepting client." << std::endl;
		return;
	}

	Client *client = new Client(client_socket, "");
	this->clients[client_socket] = client;
	std::cout << "Client connected." << std::endl;
}

void Server::removeClient(Client *client)
{
	this->clients.erase(client->getSocketFd());
	close(client->getSocketFd());
	std::cout << "Client removed." << std::endl;
}

void Server::handleConnections()
{
	for (std::map<int, Client*>::iterator it = clients.begin(); it != clients.end(); ++it)
	{
		int client_fd = it->first;
		char buffer[1024];
		int bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
		std::cout << "Bytes received: " << bytes_received << std::endl;
		if (bytes_received > 0)
		{
			buffer[bytes_received] = '\0';
			std::cout << "Received message from client: " << buffer << std::endl;

			std::string message = "Message received: --->" + std::string(buffer);
			send(client_fd, message.c_str(), message.length(), 0);
		}
		else if (bytes_received == 0)
		{
			std::cout << "Client disconnected." << std::endl;
			removeClient(it->second);
		}
		else
		{
			std::cerr << "Error receiving data from client." << std::endl;
			removeClient(it->second);
		}
	}
	std::cout << "Handling connections." << std::endl;
}

bool Server::verifyPassword(const std::string &password)
{
	return (this->password == password);
}
