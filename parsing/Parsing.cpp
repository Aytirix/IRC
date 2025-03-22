#include "Parsing.hpp"
#include <string>

Parsing::Parsing(Server &server) : server(server) {}

Parsing::~Parsing() {}

/*
** split
** Split une string en fonction d'un délimiteur
** @param str : la string à split
** @param delim : le délimiteur
** @return un vector contenant les morceaux de la string
*/
std::vector<std::string> Parsing::split(const std::string &str, char delim)
{
	std::vector<std::string> elems;
	std::stringstream ss(str);
	std::string item;

	while (std::getline(ss, item, delim))
	{
		if (item.size() > 0)
			elems.push_back(item);
	}
	return elems;
}

/*
** RemoveHiddenChar
** Supprime les caractères cachés '\r' et '\n' d'une string
** @param str : la string à nettoyer
** @return la string nettoyée
*/
std::string Parsing::RemoveHiddenChar(std::string &str)
{
	if (str.find("\r") != std::string::npos)
		str.erase(str.find("\r"), 1);
	if (str.find("\n") != std::string::npos)
		str.erase(str.find("\n"), 1);
	return str;
}

/*
** init_parsing
** Initialise le parsing des commandes
** @param client : le client qui a envoyé la commande
** @param buffer : la commande à parser
*/
bool Parsing::init_parsing(Client &client, std::string &buffer)
{
	buffer = RemoveHiddenChar(buffer);
	log::write(log::RECEIVED, "fd (" + log::toString(client.getSocketFd()) + ") : '" + buffer + "'");

	if (buffer.substr(0, 3) == "CAP")
		return capability(client, buffer);
	if (std::string("PASS NICK USER").find(buffer.substr(0, 4)) != std::string::npos)
		return InitializeUser(client, buffer);
	else if (buffer.substr(0, 4) == "MODE")
	{
		std::string channel = buffer.substr(5, buffer.size() - 5);
		server.send_data(client.getSocketFd(), "324 " + client.getNickname() + " " + channel + " +itkol");
	}
	else if (buffer.substr(0, 4) == "JOIN")
	{
		std::string channel = buffer.substr(5, buffer.size() - 5);
		server.joinChannel(client, channel);
	}
	return true;
}

bool Parsing::InitializeUser(Client &client, std::string &buffer)
{
	if (buffer.substr(0, 4) == "PASS")
	{
		std::string password = buffer.substr(5, buffer.size() - 5);
		if (!Hasher::compare(password, server.password_))
		{
			log::write(log::INFO, "L'utilisateur : fd(" + log::toString(client.getSocketFd()) + ") a rentré un mauvais mot de passe");
			server.send_data(client.getSocketFd(), ERR_PASSWD_MISMATCH);
			server.DisconnectClient(client);
			return false;
		}
	}
	else if (buffer.substr(0, 4) == "USER")
	{
		if (client.getUserName().size() > 0)
		{
			server.send_data(client.getSocketFd(), ERR_ALREADY_REGISTERED(client.getNickname()));
			return true;
		}
		std::string username = buffer.substr(5, buffer.size() - 5);
		username = username.substr(0, username.find(" "));
		client.setUserName(username);
		username = buffer.substr(buffer.find(":", 0) + 1, buffer.size() - buffer.find(":", 0) - 1);
		client.setRealName(username);
	}
	else if (buffer.substr(0, 4) == "NICK")
	{
		std::string nickname = buffer.substr(5, buffer.size() - 5);
		client.setNickname(nickname);
	}
	return true;
}

/*
** capability
** Gère les commandes CAP
** Si une commande CAP LS 302 est reçue, on envoie la liste des capabilities
** Si une commande CAP REQ est reçue, on vérifie que les capabilities demandées sont bien supportées
** @param client : le client qui a envoyé la commande
** @param buffer : la commande à parser
*/
bool Parsing::capability(Client &client, std::string &buffer)
{
	std::string caps = "chghost extended-join multi-prefix account-tag cap-notify";
	if (buffer.substr(0, 10) == "CAP LS 302")
		server.send_data(client.getSocketFd(), "CAP * LS :" + caps, true, false);
	else if (buffer.substr(0, 7) == "CAP REQ")
	{
		std::string cap = buffer.substr(9, buffer.size() - 9);
		std::vector<std::string> cap_req = Parsing::split(cap, ' ');
		for (size_t i = 0; i < cap_req.size(); i++)
		{
			if (!(cap.find(cap_req[i]) != std::string::npos))
			{
				server.send_data(client.getSocketFd(), "CAP" + client.getNickname() + " NAK " + cap_req[i], true, false);
				return true;
			}
		}
		server.send_data(client.getSocketFd(), "CAP " + client.getNickname() + " ACK :" + cap, true, false);
	}
	else if (buffer.substr(0, 7) == "CAP END")
		server.send_data(client.getSocketFd(), "001 " + std::string(WELCOME(client.getNickname())), true, false);
	return true;
}
