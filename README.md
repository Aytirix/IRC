# Projet IRC - École 42

## Description

Ce projet consiste à créer un serveur IRC (Internet Relay Chat). Vous pouvez tester ce serveur en vous connectant avec un client IRC existant, comme **IRSSI**.

## Diagramme

Vous pouvez consulter le diagramme du projet en suivant [ce lien](https://app.diagrams.net/?src=about#HAytirix%2FIRC%2Fmain%2FDiagramme_IRC.drawio).

## Lancer le serveur

1. **Exécuter le serveur**

	Lancez le serveur en utilisant la commande suivante, en remplaçant `<port>` par le port d'écoute (par exemple 6667) et `<password>` par le mot de passe que vous souhaitez utiliser :

	```bash
	./ircserv <port> <password>
	```

## Se connecter avec un client IRC

1. **Lancer IRSSI** avec la commande suivante :

Cette commande se connecte à votre serveur IRC local (**localhost**), sur le port spécifié, avec le pseudo et le mot de passe spécifiés.
	```bash
	irssi -c localhost -p <port> -n <pseudo> -w <password>
	```

---
