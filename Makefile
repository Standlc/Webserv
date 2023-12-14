NAME = webserv

SOURCES = 	srcs/main.cpp \
			srcs/utils/utils.cpp \
			srcs/Server.cpp \
			\
			srcs/blocks/Block.cpp \
			srcs/blocks/ServerBlock.cpp \
			srcs/blocks/LocationBlock/LocationBlock.cpp \
			srcs/blocks/LocationBlock/utils.cpp \
			srcs/blocks/LocationBlock/serverRequests/get.cpp \
			srcs/blocks/LocationBlock/serverRequests/post.cpp \
			srcs/blocks/LocationBlock/serverRequests/delete.cpp \
			srcs/blocks/LocationBlock/serverRequests/serverRequests.cpp \
			srcs/blocks/LocationBlock/reverseProxy.cpp \
			srcs/blocks/LocationBlock/redirections.cpp \
			\
			srcs/PollEvents/PollFd.cpp \
			srcs/PollEvents/ClientPoll.cpp \
			srcs/PollEvents/CgiPoll.cpp \
			srcs/PollEvents/ProxyPoll.cpp \
			\
			srcs/ServerStreams/HttpRequest.cpp \
			srcs/ServerStreams/HttpResponse.cpp \
			srcs/ServerStreams/cgi/CgiRequest.cpp \
			srcs/ServerStreams/cgi/CgiResponse.cpp \
			srcs/ServerStreams/proxy/ProxyRequest.cpp \
			srcs/ServerStreams/proxy/ProxyResponse.cpp \
			srcs/ServerStreams/HttpParser.cpp \
			parsing.cpp fill.cpp

OBJECTS = $(SOURCES:.cpp=.o)

DEPENDENCIES = 	Makefile \
				srcs/Server.hpp \
				srcs/blocks/Block.hpp \
				srcs/PollEvents/PollFd.hpp \
				srcs/ServerStreams/HttpResponse.hpp \
				srcs/ServerStreams/HttpRequest.hpp \
				srcs/StaticClasses/MediaTypes.hpp \
				srcs/webserv.hpp \
				srcs/StaticClasses/StatusComments.hpp \
				srcs/StaticClasses/MediaTypes.hpp \
				srcs/ServerStreams/ServerStream.hpp \
				srcs/ServerStreams/cgi/CgiRequest.hpp \
				srcs/ServerStreams/cgi/CgiResponse.hpp \
				srcs/ServerStreams/proxy/ProxyRequest.hpp \
				srcs/ServerStreams/proxy/ProxyResponse.hpp \
				srcs/ServerStreams/HttpParser.hpp \
				srcs/blocks/LocationBlock/utils.hpp \

FLAGS = #-Wextra -Wall #-std=c++98 #-Werror

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