NAME = webserv

SRCS = main.cpp

OBJECTS = $(SRCS:.cpp=.o)

DEPENDENCIES = 	Server.hpp				ServerConfig.hpp \
				ServerBlock.hpp			LocationBlock.hpp \
				LocationBlock.hpp		HttpResponse.hpp \
				HttpRequest.hpp			MediaTypes.hpp \
				webserv.hpp \

RM = rm -rf

FLAGS = #-Werror -Wall -Wextra -std=c++98

all: $(NAME)

$(NAME) : $(OBJECTS)
		c++ $(OBJECTS) -o $(NAME)

%.o: %.cpp $(DEPENDENCIES)
		c++ $(CFLAGS) -c $< -o $@

clean:
		@$(RM) $(OBJECTS)

fclean:	clean
		@$(RM) $(NAME)

re : fclean all

.PHONY : all re fclean clean