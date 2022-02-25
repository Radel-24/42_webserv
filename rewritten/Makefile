NAME = webserv

CC = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -Wshadow -Wno-shadow

SRC =	bigMain.cpp		\
		Request.cpp		\
		utils.cpp


HEADER = Request.hpp utils.hpp

INC = -Iinc

SOURCES = $(addprefix src/, $(SRC))
OBJ = $(patsubst %.cpp, %.o, $(SRC))
RM = rm -rf

#build everything
all: $(NAME)

#creates programm
$(NAME): $(SOURCES) $(addprefix inc/, $(HEADER))
	$(CC) $(CPPFLAGS) $(INC) -c $(SOURCES)
	mkdir -p obj
	mv $(OBJ) obj
	$(CC) $(CPPFLAGS) $(INC) $(addprefix obj/, $(OBJ)) -o $(NAME)

clean:
	$(RM) obj

#removes executable
fclean: clean
	$(RM) $(NAME)

#removes executable and rebuild it
re: fclean all