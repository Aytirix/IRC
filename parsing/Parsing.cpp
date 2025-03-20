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
	// TYPE DE FORMAT QUAND ON UTILISE SE CONNECTE
	// Chaque ligne cmd est un nouvelle appelle a parsing
	// Nouvelle connexion : fd 4
	// cmd : fd client 4 : 'PASS test'
	// cmd : fd client 4 : 'NICK test2'
	// cmd : fd client 4 : 'USER thmouty thmouty localhost :Theo Mouty'
	// si les CAP LS donc les 6 premiers charactere sont CAP LS
	if (buffer.substr(0, 4) == "PASS")
	{
		std::string password = buffer.substr(5, buffer.size() - 5);
		std::cout << "password parsgin : '" << password << "'" << std::endl;
		if (Hasher::compare(password, server.password_))
			server.send_data(client.getSocketFd(), "001 :Bienvenue sur le serveur");
		else
		{
			log::write(log::INFO, "L'utilisateur : fd(" + log::toString(client.getSocketFd()) + ") a rentré un mauvais mot de passe");
			server.send_data(client.getSocketFd(), ERR_PASSWD_MISMATCH);
			server.DisconnectClient(client);
			return false;
		}
	}
	return true;
}