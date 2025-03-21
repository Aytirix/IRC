#include "Parsing.hpp"
#include <string>

Parsing::Parsing(Server &server) : server(server) {}

Parsing::~Parsing() {}

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
	std::cout << "cmd : fd client " << client.getSocketFd() << " : '" << buffer << "'" << std::endl;
	// TYPE DE FORMAT QUAND ON UTILISE SE CONNECTE
	// Chaque ligne cmd est un nouvelle appelle a parsing
	// Nouvelle connexion : fd 4
	// cmd : fd client 4 : 'PASS test'
	// cmd : fd client 4 : 'NICK test2'
	// cmd : fd client 4 : 'USER thmouty thmouty localhost :Theo Mouty'
	// si les CAP LS donc les 6 premiers charactere sont CAP LS
	if (buffer.substr(0, 10) == "CAP LS 302")
	{
		server.send_data(client.getSocketFd(), "CAP * LS :chghost extended-join multi-prefix account-tag cap-notify", true, false);
	}
	else if (buffer.substr(0, 7) == "CAP REQ" || buffer.substr(0, 7) == "CAP END")
	{
		static std::string cap = "";
		if (buffer.substr(0, 7) == "CAP REQ")
			cap = buffer.substr(8, buffer.size() - 8);
		else if (buffer.substr(0, 7) == "CAP END")
			server.send_data(client.getSocketFd(), " CAP thmouty1 ACK " + cap, true, false);
	}
	else if (buffer.substr(0, 4) == "PASS")
	{
		std::string password = buffer.substr(5, buffer.size() - 5);
		std::cout << "password parsgin : '" << password << "'" << std::endl;
		if (Hasher::compare(password, server.password_))
			server.send_data(client.getSocketFd(), WELCOME(client.getNickname()));
		else
		{
			log::write(log::INFO, "L'utilisateur : fd(" + log::toString(client.getSocketFd()) + ") a rentré un mauvais mot de passe");
			server.send_data(client.getSocketFd(), ERR_PASSWD_MISMATCH);
			server.DisconnectClient(client);
			return false;
		}
	}
	else if (buffer.substr(0, 4) == "MODE")
	{
		std::string channel = buffer.substr(5, buffer.size() - 5);
		server.send_data(client.getSocketFd(), "324 " + client.getNickname() + " " + channel + " +itkol");
	}
	else if (buffer.substr(0, 4) == "NICK")
	{
		std::string nickname = buffer.substr(5, buffer.size() - 5);
		client.setNickname(nickname);
	}
	else if (buffer.substr(0, 4) == "JOIN")
	{
		std::string channel = buffer.substr(5, buffer.size() - 5);
		server.joinChannel(client, channel);
	}
	return true;
}