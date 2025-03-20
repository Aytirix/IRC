
#ifndef RESPONSES_HPP
#define RESPONSES_HPP

#define SERVER_NAME "IRC_SERVER"
/* ----------------------- CMD ----------------------- */
/* --------- INVITE --------- */
/* --------- JOIN ----------- */
/* --------- KICK ----------- */
/* --------- MSG PRIVE ------ */
/* --------- PING ----------- */
/* ----------------------- PASSWORD ----------------------- */
#define WELCOME(nickname) ":" SERVER_NAME nickname " :Welcome to the IRC Network, Théo!"
#define ERR_PASSWD_MISMATCH "464 :Password incorrect"
#define ERR_PASSWD_MISSING "461 :Password required"
/* ----------------------- MODE ----------------------- */
/* ----------------------- NICK ----------------------- */
/* ----------------------- TOPIC ----------------------- */


#endif