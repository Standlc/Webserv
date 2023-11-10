NAME = webserv

SOURCES = main.cpp

OBJECTS = $(SOURCES:.cpp=.o)

DEPENDENCIES = 	Makefile Server.hpp				ServerConfig.hpp \
				ServerBlock.hpp			LocationBlock.hpp \
				LocationBlock.hpp		HttpResponse.hpp \
				HttpRequest.hpp			MediaTypes.hpp \
				webserv.hpp \

FLAGS = -Wextra -Wall -std=c++98 #-Werror 

RM = rm -rf

all: fclean $(NAME)

$(NAME): $(OBJECTS)
	c++ $(OBJECTS) -o $(NAME)

%.o: %.cpp
	c++ $(FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all re fclean clean