/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parsing.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:41:33 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 06:41:36 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"
// #include "Channel.hpp"

std::string& lefttrim(std::string& s, const char* t = " \t\n\r\f\v");
std::string& righttrim(std::string& s, const char* t = " \t\n\r\f\v");

static void splitMsg(std::vector<std::string> &cmds, std::string msg)
{
	int pos = 0;
	std::string substr;

	while ((pos = msg.find("\n")) != static_cast<int>(std::string::npos))
	{
		substr = msg.substr(0, pos);
		substr = lefttrim(substr); //removes white space in the beginning of the string
		cmds.push_back(substr);
		msg.erase(0, pos + 1);
	}
}

int parseCommand(std::string cmd_line, msg_struct &cmd_infos)
{
	if (cmd_line.empty() == true)
		return (FAILURE);
	const char* esc_chars = " \t\f\v";

	std::string copy = cmd_line;
	if (cmd_line[0] == ':') // if prefix is : delete until first space
	{
		copy.erase(0, copy.find_first_not_of(esc_chars)); //remove till occurence of 1st white space
		copy = lefttrim(copy); //remove all white spaces at beginning
		// if (cmd_line.find_first_of(' ') != std::string::npos)
		// 	copy.erase(0, copy.find_first_of(' ') + 1);
	}

	if (copy.find_first_of(esc_chars) == std::string::npos)// || copy.find_first_of('\t') == std::string::npos) // incase of command without arguments
	{
		cmd_infos.cmd.insert(0, copy, 0, std::string::npos);
		if (cmd_infos.cmd.find('\r') != std::string::npos) // delete the \r\n
			cmd_infos.cmd.erase(cmd_infos.cmd.find('\r'), 1);
	}
	else
		cmd_infos.cmd.insert(0, copy, 0, copy.find_first_of(esc_chars)); //---->put the command inside struct

	size_t prefix_length = cmd_line.find(cmd_infos.cmd, 0);
	cmd_infos.prefix.assign(cmd_line, 0, prefix_length); //---->put prefix into struct
	size_t msg_beginning = cmd_line.find(cmd_infos.cmd, 0) + cmd_infos.cmd.length() + 1;
	if (msg_beginning < cmd_line.length())
		cmd_infos.parameter = cmd_line.substr(msg_beginning, std::string::npos); //---->put pararmeter into struct
	cmd_infos.parameter = lefttrim(cmd_infos.parameter);
	cmd_infos.parameter = righttrim(cmd_infos.parameter);
	if (cmd_infos.parameter.find("\r") != std::string::npos)
		cmd_infos.parameter.erase(cmd_infos.parameter.find("\r"), 1); //---->remove \r\n

	for (size_t i = 0; i < cmd_infos.cmd.size(); i++)
		cmd_infos.cmd[i] = std::toupper(cmd_infos.cmd[i]);
	return (SUCCESS);
}

void Server::fillDetails(Client *c, int client_fd, std::string cmd)
{
	msg_struct msg_info;
	int temp;

	if (parseCommand(cmd, msg_info) == FAILURE)
		return;
	temp = (cmd.find(' ') == std::string::npos) ? 0 : cmd.find_first_of(' ');

	for (int i = 0; i < temp; i++)
		cmd[i] = std::toupper(cmd[i]);

	if (msg_info.cmd == "NICK")
		nick(this, client_fd, msg_info);
	else if (msg_info.cmd == "USER")
		user(this, client_fd, msg_info);
	else if (msg_info.cmd == "PASS")
	{
		if (pass(this, client_fd, msg_info) == SUCCESS)
			c->set_passFlag(true);
		else
			c->set_passFlag(false);
	}
}

void printRcvMsg(int fd, std::string msg)
{
	std::cout << "[Server] Message sent to client " << fd << ": " << RED << msg << RESET << "\n";
}

void sendListOfCmds(Server *s, int fd)
{
	Client *c = s->GetClient(fd);

	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "Commands Available:"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* NICK <nickname>"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* QUIT [<:reason>]"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* PASS <password>"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* PRIVMSG <user/channel> <msg>"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* JOIN <channel(s)> [<keys>]"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* MODE <channel(s)> [[+|- mode_char] parameters]"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* KICK <channel> <user> [<reason>]"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* PART <channel> [<message>]"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* INVITE <nick> <channel>"));
	s->SendReply(fd, RPL_MYINFO2(c->get_nickname(), s->GetServerName(), "	* TOPIC <channel> [<topic>]"));
}

int Server::parseMessage(int fd, std::string msg)
{
	std::vector<std::string> cmds;
	Client *c = GetClient(fd);

	splitMsg(cmds, msg);
	int size = cmds.size();
	for (int i = 0; i != size; i++)
	{
		if (c->isAuthDone() == false)
		{
			if (c->has_all_info() == false)
			{
				fillDetails(c, fd, cmds[i]);
				if (!c->get_nickname().empty() && !c->get_username().empty() && c->get_passFlag())
					c->has_all_info() = true;
			}
			if (c->has_all_info() == true && c->first_invite() == false)
			{
				this->SendReply(fd, RPL_WELCOME(this->GetServerName(), c->get_nickname()));
				this->SendReply(fd, RPL_YOURHOST(c->get_nickname(), this->GetServerName(), "1.1")); //--->get version and other details
				this->SendReply(fd, RPL_CREATED(this->GetServerName(), c->get_nickname(), this->getDate())); //---->get date in realtime and print it
				this->SendReply(fd, RPL_MYINFO(c->get_nickname(), this->GetServerName(), "1.1", c->get_nickname(), "", ""));
				sendListOfCmds(this, fd);
				printRcvMsg(fd, ": Welcome message sent...\n");

				c->first_invite() = true;
				c->isAuthDone() = true;
			}
		}
		else
		{
			//returns 1 in case of QUIT(condition added so conditional jumps can be avoided)
			if (execCommand(fd, cmds[i]) == 1) 
				return (1);
		}
	}
	return (0);
}

int Server::execCommand(int client_fd, std::string cmd_line)
{
	msg_struct cmd_infos;
	// Client *client = this->GetClient(client_fd);
	int i = 0;

	std::string validCmds[14] = {"NICK", "USER", "QUIT", "PASS", "PRIVMSG",
								 "PONG", "JOIN", "MODE", "KICK", "PART",
								 "INVITE", "TOPIC", "LIST", "NOTICE" }; //---->keep adding commands

	if (parseCommand(cmd_line, cmd_infos) == FAILURE)
		return (0);
	while (i < 14)
	{
		if (cmd_infos.cmd == validCmds[i])
			break;
		i++;
	}
	switch (i)
	{
	case 0:
		nick(this, client_fd, cmd_infos);
		break;
	case 1:
		user(this, client_fd, cmd_infos);
		break;
	case 2:
		quit(this, client_fd, cmd_infos);
		return (1);
		break;
	case 3:
		pass(this, client_fd, cmd_infos);
		break;
	case 4:
		privmsg(this, client_fd, cmd_infos);
		break;
	case 5:
		pong(this, client_fd, cmd_infos);
		break;
	case 6:
		join(this, client_fd, cmd_infos);
		break;
	case 7:
		mode(this, client_fd, cmd_infos);
		break;
	case 8:
		kick(this, client_fd, cmd_infos);
		break;
	case 9:
		part(this, client_fd, cmd_infos);
		break;
	case 10:
		invite(this, client_fd, cmd_infos);
		break;
	case 11:
		topic(this, client_fd, cmd_infos);
		break;
	case 12:
		list(this, client_fd, cmd_infos);
		break;
	case 13:
		notice(this, client_fd, cmd_infos);
		break;
	default:
		this->SendReply(client_fd, ERR_UNKNOWNCOMMAND(cmd_infos.cmd));
	}
	return (0);
}
