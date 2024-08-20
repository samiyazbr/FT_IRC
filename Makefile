# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/01/19 06:40:18 by ataro-ga          #+#    #+#              #
#    Updated: 2024/02/13 20:11:24 by hkunnam-         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	= ircserv

SRCS	= main.cpp Server.cpp Client.cpp Channel.cpp pass.cpp parsing.cpp\
		nick.cpp user.cpp quit.cpp Privmsg.cpp utils.cpp Mode.cpp pong.cpp \
		join.cpp kick.cpp part.cpp invite.cpp topic.cpp list.cpp notice.cpp

OBJS	= ${SRCS:.cpp=.o}

CXX		= c++

CXXFLAGS	=  -Werror -Wextra -Wall -std=c++98


OBJ = ${SRC:%.cpp=%.o}

all:	${NAME}

${NAME}:	${OBJS}
		$(CXX) ${CXXFLAGS} ${OBJS} -o $(NAME)

clean:		
			@rm -rf ${OBJS}

fclean:		clean
			@rm -rf ${NAME}

re:			fclean all

.PHONY: all clean fclean re
