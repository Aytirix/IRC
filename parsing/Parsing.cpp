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
		server.send_data(client.getSocketFd(), "CAP * LS :account-notify away-notify chghost extended-join multi-prefix sasl=ECDSA-NIST256P-CHALLENGE,EXTERNAL,PLAIN,SCRAM-SHA-512 tls account-tag cap-notify echo-message server-time solanum.chat/identify-msg solanum.chat/oper solanum.chat/realhost", true, false);
	}
	else if (buffer.substr(0, 7) == "CAP REQ")
	{
		// a finir
		// attendre la fin de cap req
		server.send_data(client.getSocketFd(), " CAP thmouty1 ACK :account-notify away-notify chghost extended-join multi-prefix account-tag cap-notify server-time solanum.chat/identify-msg", true, false);
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
	else if (buffer.substr(0, 4) == "JOIN")
	{
		// des tests pour essayer de comprendre mdrr
		// server.send_data(client.getSocketFd(), ":thmouty1 JOIN my_test * :realname", false, true);
		// server.send_data(client.getSocketFd(), ":thmouty1 353 thmouty1 @ #my_test :gacavali @thmouty1", false, true);
		// server.send_data(client.getSocketFd(), "366 thmouty1 #my_test :End of /NAMES list.");
	}
	return true;
}