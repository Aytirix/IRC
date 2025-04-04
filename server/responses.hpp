#ifndef RESPONSES_HPP
#define RESPONSES_HPP

#define SERVER_NAME ":irc.42project.chat "

/* ----------------------- CMD ----------------------- */
#define NOT_ENOUGH_PARAMS(nickname, cmd) "461 " + nickname + " " + cmd + " :Not enough parameters"

/* ----------------------- CHANNEL ----------------------- */
#define LEAVE_CHANNEL(unique_nickname, channel, message) ":" + unique_nickname + " PART " + channel + " :" + message
#define USER_NOT_IN_CHANNEL(nickname, channel) "442 " + nickname + " " + channel + " :You're not on that channel"
#define USER_JOIN_CHANNEL(unique_nickname, channel) ":" + unique_nickname + " JOIN " + channel
#define ERR_NOSUCH_CHANNEL(nickname, channel) "403 " + nickname + " " + channel + " :No such channel"
#define TEXT_NOT_FOUND(nickname) "412 " + nickname + " :No text to send"
/* --------- KICK ----------- */
#define NOT_OPERATOR(unique_nickname, channel) "482 " + unique_nickname + " " + channel + " :You're not a channel operator"
#define KICK(unique_nickname, channel, nickname_kick, message) ":" + unique_nickname + " KICK " + channel + " " + nickname_kick + " :" + message
#define ERR_NOSUCH_NICK(nickname, target, channel) "441 " + nickname + " " + target + " " + channel + " :They aren't on that channel"
/* --------- WHO --------- */
#define END_OF_WHO(nickname, channel) "315 " + nickname + " " + channel + " :End of /WHO list"
#define WHO_LIST_USER(nickname, channel, list_users) "353 " + nickname + " @ " + channel + " :" + list_users
#define END_OF_NAMES(nickname, channel) "366 " + nickname + " " + channel + " :End of /NAMES list"
/* ----------- TOPIC ----------- */
#define INIT_TOPIC(nickname, channel, topic) "332 " + nickname + " " + channel + " :" + topic
#define SET_TOPIC(unique_nickname, channel, topic) ":" + unique_nickname + " TOPIC " + channel + " :" + topic
#define NOT_TOPIC_SET(nickname, channel) "331 " + nickname + " " + channel + " :No topic is set."
/* --------- MSG PRIVE ------ */
#define PRIV_MSG(unique_nickname, target, message) ":" + unique_nickname + " PRIVMSG " + target + " :" + message
#define PRIV_MSG_NO_RECIPIENT(nickname) "411 " +  nickname + " :No recipient given (PRIVMSG)"
/* --------- INVITE --------- */
#define INVITE_NO_SUCH(nickname, target) "401 " + nickname + " " + target + " :No such nick/channel"
#define INVITE_TO_TARGET(unique_nickname, target, channel) ":" + unique_nickname + " INVITE " + target + " :" + channel
#define INVITE_CALLBACK(nickname, target, channel) "341 " + nickname + " " + target + " " + channel
#define ERR_USER_ON_CHANNEL(nickname, target, channel) "443 " + nickname + " " + target + " " + channel + " :is already on channel"
#define ERR_INVITE_ONLY(nickname, channel) "473 " + nickname + " " + channel + " :Cannot join channel (+i)"
/* --------- LIST --------- */
#define LIST_START(nickname) "321 " + nickname + " Channel :Users  Name"
#define LIST_CHANNEL(nickname, channel, user_connected, topic) "322 " + nickname + " " + channel + " " + user_connected + " :" + topic
#define END_OF_LIST(nickname) "323 " + nickname + " :End of /LIST"
/* -------- LIMIT -------- */
#define ERR_CHANNELISFULL(nickname, channel) "471 " + nickname + " " + channel + " :Cannot join channel (+l)"
/* -------- MODE -------- */
#define MODE_CHANNEL(nickname, channel, mode) "324 " + nickname + " " + channel + " +" + mode
#define DEFAULT_MODES(channel) "MODE " + channel + " +Cnst"
#define UPDATE_MODE(unique_nickname, channel, mode) ":" + unique_nickname + " MODE " + channel + " " + mode
/* -------- PASSWORD -------- */
#define ERR_BADCHANNELKEY(nickname, channel) "475 " + nickname + " " + channel + " :Cannot join channel (+k)"

/* ----------------------- CONNEXION ----------------------- */
#define WELCOME(nickname) "001 "+ nickname +" :Bienvenue "+ nickname +" sur le serveur 42Project !"
#define ERR_PASSWD_MISMATCH "464 :Password incorrect"
#define ERR_PASSWD_MISSING "461 :Password required"
#define ERR_ALREADY_REGISTERED(nickname) "462 " + nickname + " :You are already connected and cannot handshake again"

/* ----------------------- NICK ----------------------- */
#define ERR_NICKNAME_IN_USE(nickname, use_nickname) "433 " + nickname + use_nickname + " :Nickname is already in use"
#define ERR_NICKNAME_MISSING "431 :No nickname given"
#define ERR_ERRONEUS_NICKNAME(old_nickname, new_nickname) "432 " + old_nickname + " " + new_nickname + " :Erroneous nickname"
#define NICKNAME_CHANGED(unique_nickname, nickname) ":" + unique_nickname + " NICK :" + nickname
#define char_autorized "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]\\_^{|}"

/* ----------------------- USERNAME ----------------------- */
#define INVALID_USERNAME(ip, username_invalid) "ERROR :Closing Link: " + ip + " (Invalid username [~" + username_invalid + "])"

/* ----------------------- CAPABILITY ----------------------- */
#define CAP_LIST(list) "CAP * LS :" + list
#define ERR_CAP_INVALID(nickname, cap) "CAP " + nickname + " NAK :" + cap
#define CAP_VALID(nickname, cap) "CAP " + nickname + " ACK :" + cap

/* ----------------------- OTHER ----------------------- */
#define ERR_NOTREGISTERED() "451 * :You have not registered"
#define ERR_UNKNOWNCOMMAND(nickname, cmd) "421 " + nickname + " " + cmd + " :Unknown command"
#define PONG(nickname, args) "PONG " + nickname + " " + SERVER_NAME + ":" + args
#endif