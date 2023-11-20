NAME = webserv

SOURCES = 	main.cpp utils/utils.cpp  Server.cpp \
			blocks/Block.cpp blocks/ServerBlock.cpp blocks/LocationBlock.cpp\
			PollEvents/PollFd.cpp PollEvents/ClientPoll.cpp PollEvents/CgiPoll.cpp  \
			HttpRequest.cpp HttpResponse.cpp 

OBJECTS = $(SOURCES:.cpp=.o)

DEPENDENCIES = 	Makefile Server.hpp	blocks/Block.hpp PollEvents/PollFd.hpp \
				HttpResponse.hpp HttpRequest.hpp MediaTypes.hpp \
				webserv.hpp StatusComments.hpp MediaTypes.hpp \

FLAGS = -Wextra -Wall -std=c++98 #-Werror

RM = rm -rf

all: $(NAME)

$(NAME): $(OBJECTS) $(DEPENDENCIES)
	c++ $(OBJECTS) -o $(NAME)

%.o: %.cpp 
	c++ $(FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all re fclean clean