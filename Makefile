NAME = webserv

SOURCES = 	main.cpp utils/utils.cpp  Server.cpp \
			blocks/Block.cpp blocks/ServerBlock.cpp \
			\
			blocks/LocationBlock/LocationBlock.cpp blocks/LocationBlock/utils.cpp \
			blocks/LocationBlock/serverRequests/get.cpp blocks/LocationBlock/serverRequests/post.cpp \
			blocks/LocationBlock/serverRequests/delete.cpp blocks/LocationBlock/serverRequests/serverRequests.cpp\
			blocks/LocationBlock/reverseProxy.cpp  \
			blocks/LocationBlock/redirections.cpp  \
			\
			PollEvents/PollFd.cpp PollEvents/ClientPoll.cpp PollEvents/CgiPoll.cpp PollEvents/ProxyPoll.cpp  \
			ServerStreams/HttpRequest.cpp ServerStreams/HttpResponse.cpp  \
			ServerStreams/cgi/CgiRequest.cpp ServerStreams/cgi/CgiResponse.cpp  \
			ServerStreams/proxy/ProxyRequest.cpp ServerStreams/proxy/ProxyResponse.cpp \
			ServerStreams/HttpParser.cpp  \

OBJECTS = $(SOURCES:.cpp=.o)

DEPENDENCIES = 	Makefile Server.hpp	blocks/Block.hpp PollEvents/PollFd.hpp \
				ServerStreams/HttpResponse.hpp ServerStreams/HttpRequest.hpp MediaTypes.hpp \
				webserv.hpp StatusComments.hpp MediaTypes.hpp ServerStreams/ServerStream.hpp \
				ServerStreams/cgi/CgiRequest.hpp ServerStreams/cgi/CgiResponse.hpp  \
				ServerStreams/proxy/ProxyRequest.hpp ServerStreams/proxy/ProxyResponse.hpp \
				ServerStreams/HttpParser.hpp blocks/LocationBlock/utils.hpp \

FLAGS = -Wextra -Wall -std=c++98 #-Werror

RM = rm -rf

all: $(NAME)

$(NAME): $(OBJECTS)
	c++ $(OBJECTS) -o $(NAME)

%.o: %.cpp  $(DEPENDENCIES)
	c++ $(FLAGS) -c $< -o $@

clean:
	$(RM) $(OBJECTS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all re fclean clean