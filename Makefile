CC = g++
NAME = ircserv

CFLAGS = -std=c++98 -pedantic -Wall -Wextra -Werror -g3 -fsanitize=address

SRCS = main.cpp \
		server/server.cpp \
		client/client.cpp \
		channel/channel.cpp \
		parsing/Parsing.cpp

OBJ_DIR = obj
OBJS = $(addprefix $(OBJ_DIR)/,$(SRCS:.cpp=.o))

all: $(NAME)

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@)  # Crée le répertoire nécessaire
	$(CC) $(CFLAGS) -c $< -o $@

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)


start: all
	-fuser -k 6667/tcp
	./$(NAME) 6667 password

restart: re start

hexchat:
	nohup flatpak run io.github.Hexchat &>/dev/null &

re: fclean all

.PHONY: all clean fclean re
