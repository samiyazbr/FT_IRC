/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Command.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:37:06 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 06:37:08 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <string>
#include "Server.hpp"
#include "Channel.hpp"

#define VALID_LEN 15
class Server;

struct msg_struct
{
	std::string prefix;
	std::string cmd;
	std::string parameter;
};

int  pass(Server *server, int clifdent_fd, msg_struct msg_info);
void nick(Server *server, int fd, msg_struct msg_info);
void user(Server *server, int client_fd, msg_struct msg_info);
void quit(Server *server, int fd, msg_struct msg_info);
void pong(Server *server, int client_fd, msg_struct msg_info);
int  join(Server *server, int client_fd, msg_struct msg_info);
void kick(Server *server, int client_fd, msg_struct msg_info);
int  part(Server *server, int client_fd, msg_struct msg_info);
int  invite(Server *server, int client_fd, msg_struct msg_info);
int  topic(Server *server, int client_fd, msg_struct msg_info);
void  list(Server *server, int fd, msg_struct cmd_infos);

#endif