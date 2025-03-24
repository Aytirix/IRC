#ifndef RESPONSES_HPP
#define RESPONSES_HPP

#define SERVER_NAME ":irc.42project.chat "

/* ----------------------- CMD ----------------------- */
#define NOT_ENOUGH_PARAMS(nickname, cmd) "461 " + nickname + " " + cmd + " :Not enough parameters"

/* --------- INVITE --------- */

/* --------- JOIN ----------- */
#define LEAVE_CHANNEL(unique_nickname, channel, message) ":" + unique_nickname + " PART " + channel + " " + message
#define USER_NOT_IN_CHANNEL(nickname, channel) "442 " + nickname + " " + channel + " :You're not on that channel"
#define USER_JOIN_CHANNEL(unique_nickname, channel) ":" + unique_nickname + " JOIN " + channel
#define ERR_NOSUCHCHANNEL(nickname, channel) "403 " + nickname + " " + channel + " :No such channel"

/* --------- KICK ----------- */

/* --------- MSG PRIVE ------ */

/* --------- PING ----------- */

/* ----------------------- CONNEXION ----------------------- */
#define WELCOME(nickname) "001 "+ nickname +" :Bienvenue "+ nickname +" sur le serveur 42Project !"
#define ERR_PASSWD_MISMATCH "464 :Password incorrect"
#define ERR_PASSWD_MISSING "461 :Password required"
#define ERR_ALREADY_REGISTERED(nickname) "462 " + nickname + " :You are already connected and cannot handshake again"

/* ----------------------- MODE ----------------------- */
#define ALL_MODES(channel) "MODE " + channel + " +itkol"

/* ----------------------- NICK ----------------------- */
#define ERR_NICKNAME_IN_USE(nickname, use_nickname) "433 " + nickname + use_nickname + " :Nickname is already in use"
#define ERR_NICKNAME_MISSING "431 :No nickname given"
#define ERR_ERRONEUS_NICKNAME(old_nickname, new_nickname) "432 " + old_nickname + " " + new_nickname + " :Erroneus nickname"
#define NICKNAME_CHANGED(unique_nickname, nickname) ":" + unique_nickname + " NICK :" + nickname

/* ----------------------- TOPIC ----------------------- */

/* ----------------------- CAPABILITY ----------------------- */
#define ERR_CAP_INVALID(nickname, cap) "CAP " + nickname + " NAK :" + cap
#define CAP_VALID(nickname, cap) "CAP " + nickname + " ACK :" + cap

/* ----------------------- OTHER ----------------------- */
#define ERR_NOTREGISTERED() "451 * :You have not registered"
#define ERR_UNKNOWNCOMMAND(nickname, cmd) "421 " + nickname + " " + cmd + " :Unknown command"
#endif