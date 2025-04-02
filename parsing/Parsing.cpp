#include "Parsing.hpp"
#include <string>



static std::vector<std::string>	ft_split(std::string &buffer)
{
	std::vector<std::string>	v_buffer;
	std::string					word, temp;
	std::stringstream 			split(buffer);
	int							i = 0;
	while (split >> word)
	{
		if (i == 2 && word == ":")
			continue ;
		else if (i == 2 && word[0] == ':') {
			temp = word.substr(1);
			v_buffer.push_back(temp);
			continue ;
		}
		v_buffer.push_back(word);
		i++;
	}
	return (v_buffer);
}

Parsing::Parsing(Server &server) : server(server) {}

Parsing::~Parsing() {}



bool Parsing::init_parsing(Client &client, std::string &buffer)
{
	std::vector<std::string>	v_buffer = ft_split(buffer);
	clean_buffer(buffer);
	log::write(log::RECEIVED, "fd (" + log::toString(client.getSocketFd()) + ") : '" + buffer + "'");
	if (v_buffer[0] == "CAP")
		capabilities(client, v_buffer);
	else if (v_buffer[0] == "PASS")
		return (pass(client, v_buffer));
	else if (v_buffer[0] == "USER")
			user(client, v_buffer);
	else if (v_buffer[0] == "NICK")
		nick(client, v_buffer);
	//USER
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

void	Parsing::capabilities(Client &client, std::vector<std::string> &v_buffer)
{
	if (v_buffer.size() == 1) 
		server.send_data(client.getSocketFd(), ERR_PARAM(client.getNickname(), v_buffer[0]), true, false);
	else if (v_buffer[1] == "LS")
	{
		std::string str = "CAP * LS :chghost";
		server.send_data(client.getSocketFd(), str, true, false);
	}
	else if (v_buffer[1] == "REQ")
	{
		if (v_buffer.size() == 2 || (v_buffer.size() <= 3 && v_buffer[2] == ":"))
			return ;
		else if (v_buffer[2] == "chghost" && v_buffer.size() < 4){
			server.send_data(client.getSocketFd(), CAP_VALID(client.getNickname(), v_buffer[2]), true, false);
		}
		else {
			std::string str;
			for (size_t i = 2; i < v_buffer.size(); i++) {
					str = str + " " +v_buffer[i];
			}
			server.send_data(client.getSocketFd(), ERR_CAP_INVALID(client.getNickname(), str), true, false);
		}
	}
}

bool	Parsing::pass(Client &client, std::vector<std::string> &v_buffer)
{
	if (client.IsConnected() == true) {
		server.send_data(client.getSocketFd(), ERR_ALREADY_REGISTERED(client.getNickname()), true, false);
	}
	else if (v_buffer.size() == 1)
		server.send_data(client.getSocketFd(), ERR_PASSWD_MISSING, true, false);
	else if (v_buffer.size() > 2)
		server.send_data(client.getSocketFd(), ERR_NEEDMOREPARAMS(client.getNickname()), true, false);
	else if (client.IsConnected() == false)
	{
		if (Hasher::compare(v_buffer[1], server.password_) == true)
			client.passwordVerified();
		else {
			server.send_data(client.getSocketFd(), ERR_PASSWD_MISMATCH, true, false);
			server.DisconnectClient(client);
			return (false);
		}
	}
	return (true);
}

void	Parsing::nick(Client &client, std::vector<std::string> &v_buffer)
{

	//nickname tolower / si il existe / 
	if (v_buffer.size() > 2)
		server.send_data(client.getSocketFd(), ERR_NEEDMOREPARAMS(client.getNickname()), true, false);
	else if (v_buffer.size() == 1) {
		server.send_data(client.getSocketFd(), ERR_NONICKNAMEGIVEN, true, false);
	}
	else if (std::string("0123456789!\"#$%&'()*+,./:;<=>?@ ~").find(v_buffer[1][0]) != std::string::npos
			|| v_buffer[1].size() > 9)
		server.send_data(client.getSocketFd(), ERR_ERRONEUSNICKNAME(client.getNickname()), true, false);
	else if (v_buffer.size() == 2) {
		// ne pas oubier de verrifier si le username est enregistrer .
		if (!client.getUserName().empty()) {
			server.send_data(client.getSocketFd(), NICKNAME_CHANGED(client.getUniqueName(), v_buffer[1]), true, false);
			server.send_data(client.getSocketFd(), WELCOME(v_buffer[1]), true, false);
		}
		client.setNickname(v_buffer[1]);
	}
}
void	Parsing::user(Client &client, std::vector<std::string> &v_buffer)
{
	client.setUserName(v_buffer[1]);
	if (client.IsConnected() == true)
		server.send_data(client.getSocketFd(), ERR_ALREADY_CONNECTED(v_buffer[1]), true, false);
}

