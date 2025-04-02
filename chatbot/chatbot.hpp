#ifndef CHATBOT_HPP
#define CHATBOT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <curl/curl.h>
#include <cstdlib>
#include <cstdio>
#include <poll.h>

#include "../server/server.hpp"
#include "../client/client.hpp"

// Structure pour représenter un message
struct Message
{
	std::string role;
	std::string content;
};

class Client;

class Chatbot : public Client
{
public:
	Chatbot(Server &server);

	~Chatbot();

	// Envoie un message utilisateur et retourne la réponse de l'assistant
	std::string sendMessage(Client *client, const std::string &userInput);
	bool addClient(Client *client);
	bool deleteClient(Client *client);

private:
	Server &server;
	// theo.mouty@outlook.fr : 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
	// aytirix@hotmail.com : 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
	std::map<int, std::vector<Message> > _clients;
	std::string apiToken;
	std::string url;
	CURL *curl;

	// Échappe les caractères spéciaux pour JSON
	std::string escapeJson(const std::string &s);

	// Construit le payload JSON à partir de l'historique
	std::string buildJsonPayload(std::vector<Message> &conversation);

	// Fonction de rappel pour écrire la réponse dans une chaîne
	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output);

	// Extrait le contenu de la réponse de l'assistant
	std::string extractAssistantResponse(const std::string &responseData);

	// Supprime les messages les plus anciens pour limiter la taille de la conversation
	void trimConversation(std::vector<Message> &conversation);

	std::vector<Message> &operator[](Client *client);
};

#endif