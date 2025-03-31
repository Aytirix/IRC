#include "Parsing.hpp"

#include "../channel/channel.hpp"
#include "../server/server.hpp"
#include "../server/responses.hpp"
#include "../server/hasher.hpp"
#include "../log/log.hpp"
#include "../chatbot/chatbot.hpp"

Parsing::Parsing(Server &server) : server(server) {}

Parsing::~Parsing() {}

/**
 * @brief toLower
 * Convertit une string en minuscules
 * @param str : la string à convertir
 * @return la string convertie
 **/
std::string Parsing::toLower(std::string str)
{
	std::string lower;
	if (str.empty())
		return str;
	for (std::size_t i = 0; i < str.length(); ++i)
		lower += std::tolower(str[i]);
	return lower;
}

/**
 * @brief split
 * Split une string en fonction d'un délimiteur
 * @param str : la string à split
 * @param delim : le délimiteur
 * @return un vector contenant les morceaux de la string
 **/
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

/**
 * @brief RemoveHiddenChar
 * Supprime les caractères cachés '\r' et '\n' d'une string
 * @param str : la string à nettoyer
 * @return la string nettoyée
 **/
std::string Parsing::RemoveHiddenChar(std::string &str)
{
	while (str[0] == ' ')
		str.erase(0, 1);
	if (str.find("\r") != std::string::npos)
		str.erase(str.find("\r"), 1);
	if (str.find("\n") != std::string::npos)
		str.erase(str.find("\n"), 1);

	// Supprime les espaces en trop entre la commande et le premier argument
	size_t space_pos = str.find(" ");
	if (space_pos == std::string::npos)
		return str;
	while (space_pos + 1 < str.size() && str[space_pos + 1] == ' ')
		str.erase(space_pos + 1, 1);
	return str;
}

/**
 * @brief check_enough_params
 * Vérifie si une commande a assez de paramètres
 * @param client : le client qui a envoyé la commande
 * @param buffer : la commande à vérifier
 * @return true si la commande a assez de paramètres, false sinon
 **/
bool Parsing::check_enough_params(Client &client, std::string &command, std::string &args)
{
	std::map<std::string, std::size_t> params;
	// COMMAND | ARGUMENTS MINIMUM POUR CETTE COMMANDE
	params["JOIN"] = 1;
	params["PART"] = 1;
	params["WHO"] = 1;
	params["PRIVMSG"] = 1;
	params["NICK"] = 1;
	params["USER"] = 1;
	params["PASS"] = 1;
	params["KICK"] = 2;
	params["TOPIC"] = 1;
	params["INVITE"] = 2;

	std::map<std::string, std::size_t>::iterator it = params.find(command);
	if (it == params.end())
		return true;

	std::vector<std::string> args_split = split(args, ' ');

	if (args_split.size() < it->second)
	{
		if (command == "PRIVMSG")
			server.send_data(client.getSocketFd(), PRIV_MSG_NO_RECIPIENT(client.getNickname()));
		else
			server.send_data(client.getSocketFd(), NOT_ENOUGH_PARAMS(client.getNickname(), command));
		return false;
	}
	return true;
}

/**
 * @brief init_parsing
 * Initialise le parsing des commandes
 * @param client : le client qui a envoyé la commande
 * @param buffer : la commande à parser
 **/
bool Parsing::init_parsing(Client &client, std::string &buffer)
{
	buffer = RemoveHiddenChar(buffer);
	if (buffer.size() == 0)
		return true;
	log::write(log::RECEIVED, "fd(" + log::toString(client.getSocketFd()) + ") : '" + buffer + "'");

	std::string command = buffer.substr(0, buffer.find(" "));
	std::string args = "";
	if (buffer.find(" ") != std::string::npos)
		args = buffer.substr(buffer.find(" ") + 1, buffer.size() - 1);

	if (!check_enough_params(client, command, args))
		return true;

	// Commande a executer quand l'utilisateur est connecté ou pas
	if (command == "CAP")
		CMD_CAP(client, args);
	else if (command == "USER")
		return CMD_USER(client, args);
	else if (command == "PASS")
		return CMD_PASS(client, args);
	else if (command == "NICK")
		CMD_NICK(client, args);
	else if (command == "QUIT")
		server.DisconnectClient(client, args);
	else if (client.IsConnected() == true)
	{
		if (command == "MODE")
			server.send_data(client.getSocketFd(), ALL_MODES(command));
		else if (command == "JOIN")
			this->CMD_JOIN(client, args);
		else if (buffer.substr(0, 3) == "WHO")
			this->CMD_WHO(client, args);
		else if (command == "PART")
			this->CMD_PART(client, args);
		else if (command == "PRIVMSG")
			this->CMD_PRIVMSG(client, args);
		else if (command == "KICK")
			this->CMD_KICK(client, args);
		else if (command == "TOPIC")
			this->CMD_TOPIC(client, args);
		else if (command == "INVITE")
			this->CMD_INVITE(client, args);
		else
			server.send_data(client.getSocketFd(), ERR_UNKNOWNCOMMAND(client.getNickname(), buffer.substr(0, buffer.find(" "))));
	}
	else
		server.send_data(client.getSocketFd(), ERR_NOTREGISTERED());
	return true;
}

/**
 * @brief CMD_WHO
 * Cette commande permet de lister les utilisateurs d'un channel
 * @param client : le client qui a envoyé la commande
 * @param buffer : la commande à parser
 **/
void Parsing::CMD_WHO(Client &client, std::string &channel)
{
	std::map<std::string, Channel>::iterator it = server._channels.find(channel);
	if (it == server._channels.end())
	{
		server.send_data(client.getSocketFd(), END_OF_WHO(client.getNickname(), channel));
		return;
	}
	std::string list_users = it->second.getAllClientsString();
	server.send_data(client.getSocketFd(), WHO_LIST_USER(client.getNickname(), channel, list_users));
	server.send_data(client.getSocketFd(), END_OF_NAMES(client.getNickname(), channel));
}

/**
 * @brief partChannel
 * Gère la commande PART
 * Si le client est dans le channel, on l'enlève et on envoie un message à tous les clients du channel
 * Si le client n'est pas dans le channel, on envoie un message d'erreur
 * @param client : le client qui a envoyé la commande
 * @param channelName : le nom du channel
 * @param message : le message à envoyer
 **/
void Parsing::CMD_PART(Client &client, std::string &args)
{
	std::string channelName = args.substr(0, args.find(" "));
	size_t colon_pos = args.find(":");
	std::string message = (colon_pos != std::string::npos) ? args.substr(colon_pos + 1, args.size() - colon_pos) : "";

	if (channelName[0] != '#')
	{
		server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), channelName));
		return;
	}

	for (std::map<std::string, Channel>::iterator it = server._channels.begin(); it != server._channels.end(); ++it)
	{
		if (it->first == channelName)
		{
			if (it->second.disconnectClientChannel(client) == false)
				break;
			it->second.broadcastMessage(LEAVE_CHANNEL(client.getUniqueName(), channelName, message));
			if (it->second.getClientCount() == 0)
			{
				log::log::write(log::log::INFO, "Suppression du channel : " + it->first);
				server._channels.erase(it);
			}
			return;
		}
	}
	server.send_data(client.getSocketFd(), USER_NOT_IN_CHANNEL(client.getNickname(), channelName));
}

/**
 * @brief Gère la commande JOIN
 * Cette commande permet de rejoindre un channel
 * @param client : le client qui a envoyé la commande
 * @param channelName : le nom du channel
 **/
void Parsing::CMD_JOIN(Client &client, std::string &channelName)
{
	if (channelName[0] != '#')
	{
		server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), channelName));
		return;
	}
	std::map<std::string, Channel>::iterator it = server._channels.find(channelName);
	if (it == server._channels.end())
	{
		Channel channel(server, channelName, client);
		server._channels.insert(std::make_pair(channelName, channel));
	}
	else
		it->second.addClient(client);
}

/**
 * @brief Gère la commande TOPIC
 * Cette commande permet de kicker un client d'un channel
 * @param client : le client qui a envoyé la commande
 * @param args : les arguments de la commande
 **/
void Parsing::CMD_KICK(Client &client, std::string &args)
{
	std::string channelName = args.substr(0, args.find(" "));
	size_t firstSpace = args.find(" ");
	std::string clientName = args.substr(args.find(" ") + 1, args.find(" ", firstSpace + 1) - firstSpace - 1);
	std::string message = (args.find(":") != std::string::npos) ? args.substr(args.find(":") + 1, args.size() - args.find(":") - 1) : clientName;
	std::map<std::string, Channel>::iterator it = server._channels.find(channelName);
	if (it == server._channels.end())
	{
		server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), channelName));
		return;
	}
	it->second.kickClient(client, clientName, message);
}

/**
 * @brief CMD_TOPIC
 * Cette commande permet de changer le topic d'un channel
 * @param client : le client qui a envoyé la commande
 * @param args : les arguments de la commande
 **/
void Parsing::CMD_TOPIC(Client &client, std::string &args)
{
	std::string channelName = args.substr(0, args.find(" "));
	std::string newtopic = args.substr(args.find(":") + 1, args.size() - 1);
	if (channelName[0] != '#')
		return server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), channelName));

	if (args.find(":") == std::string::npos)
		return server.send_data(client.getSocketFd(), NOT_TOPIC_SET(client.getNickname(), channelName));
	std::map<std::string, Channel>::iterator it = server._channels.find(channelName);
	if (it == server._channels.end())
		return server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), channelName));
	it->second.setTopic(client, newtopic);
}

/**
 * @brief CMD_INVITE
 * Cette commande permet d'inviter un client à rejoindre un channel
 * @param client : le client qui a envoyé la commande
 * @param args : les arguments de la commande
 **/
void Parsing::CMD_INVITE(Client &client, std::string &args)
{
	std::string channelName = args.substr(args.find(" ") + 1, args.size() - args.find(" ") - 1);
	std::string clientInvite = args.substr(0, args.find(" "));
	log::write(log::DEBUG, "Channel : '" + channelName + "'");
	log::write(log::DEBUG, "Invite : '" + clientInvite + "'");
	if (channelName[0] != '#')
		return server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), channelName));

	// Recherche le channel
	std::map<std::string, Channel>::iterator it_channel = server._channels.find(channelName);
	if (it_channel == server._channels.end())
		return server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), channelName));

	// Recherche le client à inviter
	std::map<int, Client>::iterator it_client = server._clients.begin();
	for (; it_client != server._clients.end(); ++it_client)
	{
		if (it_client->second.getNickname() == clientInvite)
			break;
	}

	// Si le client n'est pas trouvé
	if (it_client == server._clients.end())
	{
		server.send_data(client.getSocketFd(), INVITE_NO_SUCH(client.getNickname(), clientInvite));
		return;
	}

	it_channel->second.sendInvite(client, it_client->second);
}

/**
 * @brief CMD_CAP
 * Gère les commandes CAP
 * Si une commande CAP LS 302 est reçue, on envoie la liste des capabilities
 * Si une commande CAP REQ est reçue, on vérifie que les capabilities demandées sont bien supportées
 * @param client : le client qui a envoyé la commande
 * @param args : la commande à parser
 **/
void Parsing::CMD_CAP(Client &client, std::string &args)
{
	std::string caps = "chghost";
	if (args == "LS 302")
		server.send_data(client.getSocketFd(), CAP_LIST(caps), true, false);
	else if (args == "REQ")
	{
		std::string cap = args.substr(5, args.size() - 5);
		std::vector<std::string> cap_req = Parsing::split(cap, ' ');
		for (size_t i = 0; i < cap_req.size(); i++)
		{
			if (caps.find(cap_req[i]) == std::string::npos)
			{
				server.send_data(client.getSocketFd(), ERR_CAP_INVALID(client.getNickname(), args.substr(5, args.size() - 5)), true, false);
				return;
			}
		}
		server.send_data(client.getSocketFd(), CAP_VALID(client.getNickname(), cap), true, false);
	}
	return;
}

/**
 * @brief CMD_PRIVMSG
 * Gère les messages privés
 * Si le message est pour un channel, on l'envoie à tous les clients du channel, sauf au client qui l'a envoyé
 * Si le message privé est pour le chatbot, on lui envoie la reponse de l'api
 * Si le message est pour un client, on l'envoie au client
 * @param client : le client qui a envoyé le message
 * @param buffer : le message à parser
 **/
void Parsing::CMD_PRIVMSG(Client &client, std::string &args)
{
	// Si pour le message on ne trouve pas de : et qu'il y a pas au moins un caractere après le :
	if (args.find(" :") == std::string::npos || args.find(":") == args.size() - 1)
	{
		server.send_data(client.getSocketFd(), TEXT_NOT_FOUND(client.getNickname()));
		return;
	}

	std::string target = args.substr(0, args.find(" "));
	size_t colon_pos = args.find(":");
	std::string message = (colon_pos != std::string::npos) ? args.substr(colon_pos + 1, args.size() - colon_pos) : "";

	// Si le messsage est pour un CHANNEL
	if (target[0] == '#')
	{
		std::map<std::string, Channel>::iterator it = server._channels.find(target);
		if (it == server._channels.end())
		{
			server.send_data(client.getSocketFd(), ERR_NOSUCH_CHANNEL(client.getNickname(), target));
			return;
		}
		it->second.broadcastMessage(client, PRIV_MSG(client.getUniqueName(), target, message));
		return;
	}
	// Sinon, c'est un message privé
	else
	{
		std::map<int, Client>::iterator it = server._clients.begin();

		// Si c'est un message privé au chatbot, ne pas traiter l'envoi de fichier
		if (target == server._chatbot->getNickname() && message[0] != '' && message.find("SHA-256 checksum for /") == std::string::npos)
		{
			std::string reponse = server._chatbot->sendMessage(client, message);
			server.send_data(client.getSocketFd(), PRIV_MSG(server._chatbot->getUniqueName(), client.getNickname(), reponse), false, true);
		}
		// Si c'est un message privé à un client
		else
		{
			for (; it != server._clients.end(); ++it)
			{
				if (it->second.getNickname() == target)
				{
					server.send_data(it->second.getSocketFd(), PRIV_MSG(client.getUniqueName(), target, message), false, true);
					return;
				}
			}
		}
	}
}

/**
 * @brief CMD_PASS
 * Gère la commande PASS
 * Cette commande permet de vérifier le mot de passe du client
 * @param client : le client qui a envoyé la commande
 * @param password : le mot de passe à vérifier
 * @return true si le mot de passe est correct, false sinon
 **/
bool Parsing::CMD_PASS(Client &client, std::string &password)
{
	// Si le client est déjà connecté, on lui envoie un message d'erreur
	if (client.IsConnected() == true)
	{
		server.send_data(client.getSocketFd(), ERR_ALREADY_REGISTERED(client.getNickname()));
		return true;
	}

	// Verifie si le client a bien envoyé un mot de passe
	if (!Hasher::compare(password, server._password))
	{
		log::write(log::INFO, "L'utilisateur : fd(" + log::toString(client.getSocketFd()) + ") a rentré un mauvais mot de passe");
		server.send_data(client.getSocketFd(), ERR_PASSWD_MISMATCH);
		server.DisconnectClient(client);
		return false;
	}
	client.passwordVerified(); // Valider le mot de passe
	return true;
}

/**
 * @brief CMD_PASS
 * Gère la commande PASS
 * Cette commande permet d'initialiser le nom d'utilisateur et le nom réel du client
 * @param client : le client qui a envoyé la commande
 * @param username : le nom d'utilisateur et le nom réel du client
 * @return true si le nom d'utilisateur est correct, false sinon
 **/
bool Parsing::CMD_USER(Client &client, std::string &username)
{
	// Si le client est déjà connecté, on lui envoie un message d'erreur
	if (client.IsConnected() == true)
	{
		server.send_data(client.getSocketFd(), ERR_ALREADY_REGISTERED(client.getNickname()));
		return true;
	}

	std::string name = username.substr(0, username.find(" "));
	// S'il commence par un caractère non autorisé
	// S'il veut prendre le nom du chatbot
	// S'il contient des caractères interdit
	// S'il fait moins de 3 caractères
	// S'il fait plus de 9 caractères
	if (std::string("0123456789!@#$%^&*()+[]\\_^{|}").find(name[0]) != std::string::npos ||
		name.find_first_of("!@#$%^&*()+[]\\^{|}") != std::string::npos ||
		name.size() < 3 ||
		name.size() > 30)
	{
		std::string old_nick = client.getNickname() != "" ? client.getNickname() : "";
		server.send_data(client.getSocketFd(), INVALID_USERNAME(client.getIp(), name), false, true);
		return false;
	}

	// Verifie si le client a bien envoyé un nom d'utilisateur
	client.setUserName(name);
	username = username.substr(username.find(":") + 1, username.size() - username.find(":") - 1);
	client.setRealName(username);
	return true;
}

/**
 * @brief Gère la commande NICK pour changer le pseudonyme d'un client.
 *
 * Cette fonction vérifie si le nouveau pseudonyme est valide et disponible.
 * Si le pseudonyme est déjà utilisé par un autre client, une erreur est renvoyée.
 * Si le pseudonyme est vide ou correspond au nom du chatbot, une erreur est renvoyée.
 * Si le changement de pseudonyme est valide, le pseudonyme du client est mis à jour
 * et un message de bienvenue est envoyé si le client n'avait pas de pseudonyme auparavant.
 *
 * @param client Référence vers l'objet Client qui souhaite changer de pseudonyme.
 * @param buffer Référence vers la chaîne de caractères contenant la commande et le nouveau pseudonyme.
 */
void Parsing::CMD_NICK(Client &client, std::string &nickname)
{
	if (client.getNickname() == nickname)
		return;

	// Si le nickname est déjà utilisé, insensible à la case
	std::string tmp_nickname = this->toLower(nickname);
	for (std::map<int, Client>::iterator it = server._clients.begin(); it != server._clients.end(); ++it)
	{
		if (it->second.getNickname() != "" && this->toLower(it->second.getNickname()) == tmp_nickname && it->second.getSocketFd() != client.getSocketFd())
		{
			std::string nick = client.getNickname() != "" ? client.getNickname() + " " : "";
			server.send_data(client.getSocketFd(), ERR_NICKNAME_IN_USE(nick, nickname));
			return;
		}
	}

	// S'il commence par un caractère non autorisé
	// S'il veut prendre le nom du chatbot
	// S'il contient des caractères interdit
	// S'il fait plus de 9 caractères
	if (std::string("0123456789!@#$%^&*()[]\\_^{|}").find(nickname[0]) != std::string::npos ||
		server._chatbot->getNickname() == nickname ||
		nickname.find_first_of("!@#$%^&*()+") != std::string::npos ||
		nickname.size() > 9)
	{
		std::string old_nick = client.getNickname() != "" ? client.getNickname() : "";
		server.send_data(client.getSocketFd(), ERR_ERRONEUS_NICKNAME(old_nick, nickname));
		return;
	}

	std::string tmp = client.getNickname();
	server.send_data(client.getSocketFd(), NICKNAME_CHANGED(client.getUniqueName(), nickname), false, true);
	client.setNickname(nickname);
	// Si le client n'a pas encore de pseudo, on lui envoie un message de bienvenue
	if (tmp.size() == 0)
	{
		// std::string str = client.getNickname() + " :Salut mon ami, je suis le chatbot du serveur, si tu as besoin d'aide, n'hésite pas à me demander !";
		// CMD_PRIVMSG(*server._chatbot, str);
		server.send_data(client.getSocketFd(), WELCOME(client.getNickname()), true, false);
	}
}
