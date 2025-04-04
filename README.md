# Projet IRC - École 42

## Description

Ce projet consiste à créer un serveur IRC (Internet Relay Chat). Vous pouvez tester ce serveur en vous connectant avec un client IRC existant, comme **IRSSI**.

## Figma
Vous pouvez consulter le Figma du projet en suivant [ce lien](https://www.figma.com/board/TEviHsREY663xd6BxfD1UQ/ft_irc?node-id=0-1&p=f).

## Lancer le serveur

1. **Exécuter le serveur**

	Lancez le serveur en utilisant la commande suivante :

	```bash
	./ircserv <port> <password>
	```

## Se connecter avec un client IRC

1. **Installer Hexchat** avec la commande suivante :

	Installer le client Hexchat :

	```bash
	flatpak remote-add --user flathub https://flathub.org/repo/flathub.flatpakrepo
 	flatpak --user install flathub io.github.Hexchat
 	flatpak run io.github.Hexchat
	```

 	```bash
	nc localhost <port>
	```

---
