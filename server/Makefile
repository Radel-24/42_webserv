NAME = webserv

CC = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98
# -Wshadow -Wno-shadow

SRC =	main.cpp		\
		Request_read.cpp		\
		Request_write.cpp		\
		utils.cpp		\
		PostResponder.cpp \
		Config.cpp		\
		Server.cpp		\
		Cgi.cpp			\
		writeStatus.cpp


HEADER = Request.hpp utils.hpp PostResponder.hpp Cgi.hpp

INC = -Iinc

SOURCES = $(addprefix src/, $(SRC))
OBJ = $(patsubst %.cpp, %.o, $(SRC))
RM = rm -rf

#build everything
all: $(NAME)

#creates programm
$(NAME): $(SOURCES) $(addprefix inc/, $(HEADER))
	make -C ../tree-2.0.2
	cp ../tree-2.0.2/tree .
	$(CC) $(CPPFLAGS) $(INC) -c $(SOURCES)
	mkdir -p obj
	mv $(OBJ) obj
	$(CC) $(CPPFLAGS) $(INC) $(addprefix obj/, $(OBJ)) -o $(NAME)

clean:
	make -C ../tree-2.0.2 clean
	$(RM) www/server*
	$(RM) obj
	$(RM) tree
	$(RM) files/*
	$(RM) binary.file

#removes executable
fclean: clean
	$(RM) $(NAME)

#removes executable and rebuild it
re: fclean all
