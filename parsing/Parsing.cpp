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
	while (str[0] == ' ')
		str.erase(0, 1);
	if (str.find("\r") != std::string::npos)
		str.erase(str.find("\r"), 1);
	if (str.find("\n") != std::string::npos)
		str.erase(str.find("\n"), 1);
	return str;
}

/*
** check_enough_params
** Vérifie si une commande a assez de paramètres
** @param client : le client qui a envoyé la commande
** @param buffer : la commande à vérifier
** @return true si la commande a assez de paramètres, false sinon
*/
bool Parsing::check_enough_params(Client &client, std::string &buffer)
{
	size_t space_pos = buffer.find(" ");

	if (space_pos == std::string::npos)
	{
		server.send_data(client.getSocketFd(), NOT_ENOUGH_PARAMS(client.getNickname(), buffer));
		return false;
	}

	// Laisser juste une espace entre le premier paramètre et le premier argument
	while (space_pos + 1 < buffer.size() && buffer[space_pos + 1] == ' ')
		buffer.erase(space_pos + 1, 1);

	if (space_pos + 1 == buffer.size())
	{
		server.send_data(client.getSocketFd(), NOT_ENOUGH_PARAMS(client.getNickname(), buffer));
		return false;
	}
	return true;
}

/*
** IsRegistered
** Vérifie si le client est bien enregistré, si ce n'est pas le cas,
** et que le client envoie une commande qui nécessite d'être enregistré,
** on envoie un message d'erreur
** @param client : le client à vérifier
** @param buffer : la commande à vérifier
** @return true si le client est enregistré, false sinon
*/
bool Parsing::IsRegistered(Client &client, std::string &buffer)
{
	if (std::string("PASS NICK USER QUIT").find(buffer.substr(0, 4)) == std::string::npos && std::string("CAP").find(buffer.substr(0, 3)) == std::string::npos)
	{
		if (client.getNickname().size() == 0)
		{
			server.send_data(client.getSocketFd(), ERR_NOTREGISTERED());
			return false;
		}
	}
	return true;
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

	if (!IsRegistered(client, buffer))
		return true;

	if (buffer.substr(0, 3) == "CAP")
		return capability(client, buffer);
	else if (std::string("PASS NICK USER").find(buffer.substr(0, 4)) != std::string::npos)
		return InitializeUser(client, buffer);
	else if (buffer.substr(0, 4) == "MODE")
	{
		if (!check_enough_params(client, buffer))
			return true;
		std::string channel = buffer.substr(5, buffer.size() - 5);
		server.send_data(client.getSocketFd(), ALL_MODES(channel));
	}
	else if (buffer.substr(0, 4) == "JOIN")
	{
		if (!check_enough_params(client, buffer))
			return true;
		std::string channel = buffer.substr(5, buffer.size() - 5);
		this->joinChannel(client, channel);
	}
	else if (buffer.substr(0, 3) == "WHO")
		this->Who(client, buffer);
	else if (buffer.substr(0, 4) == "PART")
	{
		if (!check_enough_params(client, buffer))
			return true;
		std::string channel = buffer.substr(5, buffer.find(" ", 5) - 5);
		size_t colon_pos = buffer.find(":");
		std::string message = (colon_pos != std::string::npos) ? buffer.substr(colon_pos, buffer.size() - colon_pos) : "";
		this->partChannel(client, channel, message);
	}
	else if (buffer.substr(0, 4) == "QUIT")
	{
		std::string message = buffer.substr(5, buffer.size() - 5);
		server.DisconnectClient(client, message);
	}
	else if (buffer.substr(0, 7) == "PRIVMSG")
	{
		if (!check_enough_params(client, buffer))
			return true;
		this->PRIVMSG(client, buffer);
	}
	else
		server.send_data(client.getSocketFd(), ERR_UNKNOWNCOMMAND(client.getNickname(), buffer.substr(0, buffer.find(" ", 0))));
	return true;
}

void Parsing::Who(Client &client, std::string &buffer)
{
	if (!check_enough_params(client, buffer))
		return;
	std::string channel = buffer.substr(4, buffer.find(" ", 4) - 4);
	std::cout << "WHO " << channel << std::endl;
	std::map<std::string, Channel>::iterator it = server.channels_.find(channel);
	if (it == server.channels_.end())
	{
		server.send_data(client.getSocketFd(), "315 " + client.getNickname() + " " + channel + " :End of /WHO list");
		return;
	}
	std::string list_users = it->second.getAllClientsString();
	server.send_data(client.getSocketFd(), "353 " + client.getNickname() + " @ " + channel + " :" + list_users);
	server.send_data(client.getSocketFd(), "366 " + client.getNickname() + " " + channel + " :End of /NAMES list");
}

void Parsing::partChannel(Client &client, std::string &channelName, std::string &message)
{
	for (std::map<std::string, Channel>::iterator it = server.channels_.begin(); it != server.channels_.end(); ++it)
	{
		if (it->first == channelName)
		{
			if (it->second.removeClient(client) == false)
				break;
			it->second.broadcastMessage(LEAVE_CHANNEL(client.getUniqueName(), channelName, message));
			return;
		}
	}
	server.send_data(client.getSocketFd(), USER_NOT_IN_CHANNEL(client.getNickname(), channelName));
}

void Parsing::joinChannel(Client &client, std::string &channelName)
{
	if (channelName[0] != '#')
	{
		server.send_data(client.getSocketFd(), ERR_NOSUCHCHANNEL(client.getNickname(), channelName));
		return;
	}
	std::map<std::string, Channel>::iterator it = server.channels_.find(channelName);
	if (it == server.channels_.end())
	{
		Channel channel(server, channelName, client);
		server.channels_.insert(std::make_pair(channelName, channel));
	}
	else
		it->second.addClient(client);
	server.send_data(client.getSocketFd(), USER_JOIN_CHANNEL(client.getUniqueName(), channelName), false, true);
}

bool Parsing::InitializeUser(Client &client, std::string &buffer)
{
	if (buffer.substr(0, 4) == "PASS")
	{
		if (client.getUserName().size() > 0)
		{
			server.send_data(client.getSocketFd(), ERR_ALREADY_REGISTERED(client.getNickname()));
			return true;
		}
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

		if (client.getNickname() == nickname)
			return true;

		// Si le nickname est déjà utilisé
		for (std::map<int, Client>::iterator it = server.clients_.begin(); it != server.clients_.end(); ++it)
		{
			if (it->second.getNickname() != "" && it->second.getNickname() == nickname && it->second.getSocketFd() != client.getSocketFd())
			{
				std::string nick = client.getNickname() != "" ? client.getNickname() + " " : "";
				server.send_data(client.getSocketFd(), ERR_NICKNAME_IN_USE(nick, nickname));
				return true;
			}
		}

		if (nickname.find("#") != std::string::npos || nickname == "chatbot")
		{
			std::string old_nick = client.getNickname() != "" ? client.getNickname() : "";
			server.send_data(client.getSocketFd(), ERR_ERRONEUS_NICKNAME(old_nick, nickname));
			return true;
		}

		std::string tmp = client.getNickname();
		server.send_data(client.getSocketFd(), NICKNAME_CHANGED(client.getUniqueName(), nickname), false, true);
		client.setNickname(nickname);
		if (tmp.size() == 0)
		{
			std::string str = "PRIVMSG " + client.getNickname() + " :Salut mon ami, je suis le chatbot du serveur, si tu as besoin d'aide, n'hésite pas à me demander !";
			PRIVMSG(*server.chatbot_, str);
			server.send_data(client.getSocketFd(), WELCOME(client.getNickname()), true, false);
		}
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
	std::string caps = "chghost";
	if (buffer.substr(0, 10) == "CAP LS 302")
		server.send_data(client.getSocketFd(), "CAP * LS :" + caps, true, false);
	else if (buffer.substr(0, 7) == "CAP REQ")
	{
		std::string cap = buffer.substr(9, buffer.size() - 9);
		std::vector<std::string> cap_req = Parsing::split(cap, ' ');
		for (size_t i = 0; i < cap_req.size(); i++)
		{
			if (caps.find(cap_req[i]) == std::string::npos)
			{
				server.send_data(client.getSocketFd(), ERR_CAP_INVALID(client.getNickname(), buffer.substr(9, buffer.size() - 9)), true, false);
				return true;
			}
		}
		server.send_data(client.getSocketFd(), CAP_VALID(client.getNickname(), cap), true, false);
	}
	return true;
}

void Parsing::PRIVMSG(Client &client, std::string &buffer)
{
	std::string channel = buffer.substr(8, buffer.find(" ", 8) - 8);
	std::string message = buffer.substr(buffer.find(":", 0) + 1, buffer.size() - buffer.find(":", 0) - 1);
	if (channel[0] == '#')
	{
		std::map<std::string, Channel>::iterator it = server.channels_.find(channel);
		if (it == server.channels_.end())
		{
			server.send_data(client.getSocketFd(), ERR_NOSUCHCHANNEL(client.getNickname(), channel));
			return;
		}
		it->second.broadcastMessage(":" + client.getUniqueName() + " PRIVMSG " + channel + " :" + message);
		return;
	}
	else
	{
		std::string target = buffer.substr(8, buffer.find(" ", 8) - 8);
		std::string message = buffer.substr(buffer.find(":", 0) + 1, buffer.size() - buffer.find(":", 0) - 1);
		std::map<int, Client>::iterator it = server.clients_.begin();

		// Si c'est un message privé au chatbot
		if (target == server.chatbot_->getNickname() && message[0] != '' && message.find("SHA-256 checksum for /") == std::string::npos)
		{
			std::string reponse = server.chatbot_->sendMessage(client, message);
			server.send_data(client.getSocketFd(), ":" + server.chatbot_->getUniqueName() + " PRIVMSG " + client.getNickname() + " :" + reponse, false, true);
		}
		else
		{
			for (; it != server.clients_.end(); ++it)
			{
				if (it->second.getNickname() == target)
				{
					server.send_data(it->second.getSocketFd(), ":" + client.getUniqueName() + " PRIVMSG " + target + " :" + message, false, true);
					return;
				}
			}
		}
	}
}