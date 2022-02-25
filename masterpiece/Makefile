CC = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -pedantic -Wshadow -Wno-shadow
RM = rm -f
SOURCES = ./main.cpp
HEADER = ./SimpleSocket.hpp ./BindingSocket.hpp ./ConnectingSocket.hpp ./ListeningSocket.hpp ./SimpleServer.hpp ./TestServer.hpp
NAME = webserv

#build everything
all: $(NAME)

#creates programm
$(NAME): $(SOURCES) $(HEADER)
	$(CC) $(CPPFLAGS) $(SOURCES) -o $(NAME)

#removes executable
clean:
	$(RM) $(NAME)

#removes executable and rebuild it
re: clean all