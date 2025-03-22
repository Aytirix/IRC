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
#define WELCOME(nickname) "Bienvenue " + nickname + " sur le serveur 42Project !"
#define ERR_PASSWD_MISMATCH "464 :Password incorrect"
#define ERR_PASSWD_MISSING "461 :Password required"
#define ERR_ALREADY_REGISTERED(nickname) "462 " + nickname + " :You are already connected and cannot handshake again"
/* ----------------------- MODE ----------------------- */
/* ----------------------- NICK ----------------------- */
/* ----------------------- TOPIC ----------------------- */
/* ----------------------- CAPABILITY ----------------------- */
#define ERR_CAP_INVALID(nickname, cap) "CAP " + nickname + " NAK :" + cap

#endif