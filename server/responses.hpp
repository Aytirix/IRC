#ifndef RESPONSES_HPP
#define RESPONSES_HPP

#define SERVER_NAME ":irc.42project.chat "
/* ----------------------- CMD ----------------------- */
/* --------- INVITE --------- */
/* --------- JOIN ----------- */
/* --------- KICK ----------- */
/* --------- MSG PRIVE ------ */
/* --------- PING ----------- */
/* ----------------------- CONNEXION ----------------------- */
#define WELCOME(nickname) "001 "+ nickname +" :Bienvenue "+ nickname +" sur le serveur 42Project !"
#define ERR_PASSWD_MISMATCH "464 :Password incorrect"
#define ERR_PASSWD_MISSING "461 :Password required"
#define ERR_NEEDMOREPARAMS(nickname) "461 " + nickname + " PASS :Not enough parameters"
#define ERR_ALREADY_REGISTERED(nickname) "462 " + nickname + " :You are already connected and cannot handshake again"
/*-------------------------USER-------------------------*/
#define ERR_ALREADY_CONNECTED(nickname) "462 " + nickname + " :You are already connected and cannot handshake again"

/* ----------------------- MODE ----------------------- */
/* ----------------------- NICK ----------------------- */
#define ERR_NONICKNAMEGIVEN "431 * :No nickname given"
#define ERR_ERRONEUSNICKNAME(nickname) "432 " + nickname + " :Erroneous nickname"
#define NICKNAME_CHANGED(unique_nickname, nickname) unique_nickname + " NICK " + nickname

/* ----------------------- TOPIC ----------------------- */
/* ----------------------- CAPABILITY ----------------------- */
#define ERR_PARAM(nickname, cap) "461 " + nickname + " " + cap + " :Not enough parameters" //moi
#define ERR_CAP_INVALID(nickname, cap) "CAP " + nickname + " NAK :" + cap //moi
#define CAP_VALID(nickname, cap) "CAP " + nickname + " ACK :" + cap

#endif

