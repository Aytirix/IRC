#ifndef CHATBOT_HPP
#define CHATBOT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <curl/curl.h>
#include <cstdlib>
#include <cstdio>

using namespace std;

// Structure pour représenter un message
struct Message
{
	string role;
	string content;
};

class Chatbot
{
public:
	Chatbot();

	~Chatbot();

	// Envoie un message utilisateur et retourne la réponse de l'assistant
	string sendMessage(const string &userInput);

private:
	// theo.mouty@outlook.fr : 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
	// aytirix@hotmail.com : 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
	// std::map<
	vector<Message> conversation;
	string apiToken;
	string url;
	CURL *curl;

	// Échappe les caractères spéciaux pour JSON
	string escapeJson(const string &s);

	// Construit le payload JSON à partir de l'historique
	string buildJsonPayload();

	// Fonction de rappel pour écrire la réponse dans une chaîne
	static size_t WriteCallback(void *contents, size_t size, size_t nmemb, string *output);

	// Extrait le contenu de la réponse de l'assistant
	string extractAssistantResponse(const string &responseData);

	// Supprime les messages les plus anciens pour limiter la taille de la conversation
	void trimConversation();
};

#endif