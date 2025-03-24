#include "Parsing.hpp"
#include <string>

Parsing::Parsing(Server &server) : server(server) {}

Parsing::~Parsing() {}



bool Parsing::init_parsing(Client &client, std::string &buffer)
{
	clean_buffer(buffer);
	log::write(log::RECEIVED, "fd (" + log::toString(client.getSocketFd()) + ") : '" + buffer + "'");
	if (buffer.substr(0, 3) == "CAP")
	{
		capabilities(client, buffer);
	}
	// if (buffer.find("CAP ls "))
	//PASS
	//NICK (check)
	//USER
	 (void)client;
	// (void)buffer;
	return (true);
}

void	Parsing::clean_buffer(std::string &buffer)
{
	size_t	pos = buffer.find("\r\n");
	if (pos != std::string::npos)
		buffer.erase(pos, 2);
	pos = buffer.find("\n");
	if (pos != std::string::npos)
		buffer.erase(pos, 1);
}

void	Parsing::capabilities(Client &client, std::string &buffer)
{
	if (buffer == "CAP LS 302")
	{
		std::string str = "CAP * LS : chghost";
		server.send_data(client.getSocketFd(), str, true, false);
	}
	else if (buffer ==  )
	{
		std::string str = "CAP  : chghost";
		server.send_data(client.getSocketFd(), str, true, false);
	}
}