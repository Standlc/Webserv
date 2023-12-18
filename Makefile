NAME					=	webserv



SRC_SERVER_METHODS		=	delete.cpp \
							get.cpp \
							post.cpp \

SRC_LOCATION_BLOCK		=	cgi.cpp \
							LocationBlock.cpp \
							redirections.cpp \
							reverseProxy.cpp \
							utils.cpp \
							${addprefix serverMethods/, ${SRC_SERVER_METHODS}} \

SRC_BLOCKS				=	Block.cpp \
							ServerBlock.cpp \
							${addprefix LocationBlock/, ${SRC_LOCATION_BLOCK}} \

SRC_CHECK				=	check_block.cpp \
							check_location.cpp \
							check_server.cpp \
							search.cpp \

SRC_FILL				=	fill_block.cpp \
							fill_location.cpp \
							fill_server.cpp \
							search.cpp \

SRC_OTHER				=	check_global.cpp \
							erase.cpp \
							error_message.cpp \
							required.cpp \
							same_server.cpp \
							utils.cpp \

SRC_PARSING				=	parsing.cpp \
							${addprefix check/, ${SRC_CHECK}} \
							${addprefix fill/, ${SRC_FILL}} \
							${addprefix other/, ${SRC_OTHER}} \

SRC_POLL_EVENTS			=	CgiPoll.cpp \
							ClientPoll.cpp \
							PollFd.cpp \
							ProxyPoll.cpp \

SRC_CGI					=	CgiRequest.cpp \
							CgiResponse.cpp \

SRC_PROXY				=	ProxyRequest.cpp \
							ProxyResponse.cpp \

SRC_SERVER_STREAMS		=	Headers.cpp \
							HttpParser.cpp \
							HttpRequest.cpp \
							HttpResponse.cpp \
							${addprefix cgi/, ${SRC_CGI}} \
							${addprefix proxy/, ${SRC_PROXY}} \

SRC_UTILS				=	SharedPtr.cpp \
							utils.cpp \

SRC						=	main.cpp \
							Server.cpp \
							${addprefix blocks/, ${SRC_BLOCKS}} \
							${addprefix parsing/, ${SRC_PARSING}} \
							${addprefix PollEvents/, ${SRC_POLL_EVENTS}} \
							${addprefix ServerStreams/, ${SRC_SERVER_STREAMS}} \
							${addprefix utils/, ${SRC_UTILS}} \

SRCS					=	${addprefix srcs/, ${SRC}} \



OBJ						=	${addprefix binaries/, ${SRC}} \



OBJECTS					=	$(OBJ:.cpp=.o) \



HEADER_UTILS			=	SharedPtr.hpp \

HEADER_STATIC_CLASSES	=	MediaTypes.hpp \
							StatusComments.hpp \

HEADER_CGI				=	CgiRequest.hpp \
							CgiResponse.hpp \

HEADER_PROXY			=	ProxyRequest.hpp \
							ProxyResponse.hpp \

HEADER_SERVER_STREAM	=	Headers.hpp \
							HttpParser.hpp \
							HttpRequest.hpp \
							HttpResponse.hpp \
							ServerStream.hpp \
							${addprefix cgi/, ${HEADER_CGI}} \
							${addprefix proxy/, ${HEADER_PROXY}} \

HEADER_POLL_EVENTS		=	PollFd.hpp \

HEADER_PARSING			=	parsing.hpp \

HEADER_LOCATION_BLOCK	=	utils.hpp \

HEADER_BLOCKS			=	Block.hpp \
							${addprefix LocationBlock/, ${HEADER_LOCATION_BLOCK}} \

HEADER_SRCS				=	Server.hpp \
							webserv.hpp \
							${addprefix blocks/, ${HEADER_BLOCKS}} \
							${addprefix parsing/, ${HEADER_PARSING}} \
							${addprefix PollEvents/, ${HEADER_POLL_EVENTS}} \
							${addprefix ServerStreams/, ${HEADER_SERVER_STREAM}} \
							${addprefix StaticClasses/, ${HEADER_STATIC_CLASS}} \
							${addprefix utils/, ${HEADER_UTILS}} \

DEPENDENCIES			=	Makefile \
							${addprefix srcs/, ${HEADER_SRCS}} \



FLAGS					=	-Wextra -Wall -Werror -std=c++98



RM						=	rm -rf



all: $(NAME)

$(NAME): $(OBJECTS)
	@c++ $(FLAGS) $(OBJECTS) -o $(NAME)
	@echo "\033[32mwebserv compiled"

binaries/%.o: srcs/%.cpp $(DEPENDENCIES)
	@mkdir -p $(@D)
	@c++ $(FLAGS) -c $< -o $@

binaries :
	@mkdir -p binaries/

clean:
	@$(RM) binaries/
	@echo "\033[36mwebserv binaries files have been deleted"

fclean: clean
	@$(RM) $(NAME)
	@echo "\033[36mwebserv file is delete"

re: fclean all

.PHONY: all re fclean clean