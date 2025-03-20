
#ifndef RESPONSES_HPP
#define RESPONSES_HPP

#define SERVER_NAME ":irc.42project.chat "
/* ----------------------- CMD ----------------------- */
/* --------- INVITE --------- */
/* --------- JOIN ----------- */
/* --------- KICK ----------- */
/* --------- MSG PRIVE ------ */
/* --------- PING ----------- */
/* ----------------------- PASSWORD ----------------------- */
#define WELCOME(nickname) "Bienvenue " + nickname + " sur le serveur 42Project !"
#define ERR_PASSWD_MISMATCH "464 :Password incorrect"
#define ERR_PASSWD_MISSING "461 :Password required"
/* ----------------------- MODE ----------------------- */
/* ----------------------- NICK ----------------------- */
/* ----------------------- TOPIC ----------------------- */


#endif