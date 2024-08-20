/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   user.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:45:06 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/12 21:42:07 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"

/* command: USER 
 * Syntax: USER <username> 0 * <realname>
is used at the beginning of a connection to 
specify the username and realname of a new user
The second and third parameters of this command SHOULD be sent as 
one zero ('0', 0x30) and one asterisk character ('*', 0x2A)
by the client, as the meaning of these two parameters varies between 
different versions of the IRC protocol.
 * 	
 * Numeric Replies:    ERR_NEEDMOREPARAMS (461)
                        ERR_ALREADYREGISTERED (462)
 */


/* Handles the findusername part
1. initialize the string user and clear to store the username.
2.	check if the input string is not empty and if the first char is a space.
	if so remove the space.
3.	if the input string is not empty , check if the string contains a space character.
	if yes extract the substring from the begining of 'msg' upto the first
	space char and assign it to the user variable. if the 'msg' dosent have 
	any space assign the entire string to the user.
4.	return user.


 */
std::string	findUsername(std::string msg)
{
	std::string user;
	user.clear();

	if (msg.empty() == false && msg[0] == ' ')
		msg.erase(0, 1);
	if (msg.empty() == false)
	{
		if (msg.find_first_of(' ') != std::string::npos)
			user.insert(0, msg, 0, msg.find(' '));
		else
			user.insert(0, msg, 0, std::string::npos);
	}
	return (user);
}

/* 
	Handles the find real name function.

1.	initialize empty string real and clear it to store the real name.
2.	check if the input string is not empty and the first char of the string
	is space. if true remove the leading space.
3.	if the input string is empty then no real name is found and empty string is returned.
4.	iterate through 'msg' count the number of spaces. if the third space char is 
	found set the begin position as current index and break the loop.
5.	if the number of spaces id more than three, extract the substring after
	the third space char to the end of the string(excluding the space) and 
	assign it to real.
6.	if the first char of real is (':') remove it from the begining.
7. return the real name.
 */
std::string	findRealname(std::string msg)
{
	std::string real;
	real.clear();

	if (msg.empty() == false && msg[0] == ' ')
		msg.erase(0, 1);
	if (msg.empty())
		return (real);
	
	int space_count = 0;
	int	begin_pos = 0;
	for (size_t i = 0; i < msg.size(); i++)
	{
		if (msg[i] == ' ')
			space_count++;
		if (space_count == 3)
		{
			begin_pos = i;
			break;
		}
	}
	if (space_count >= 3)
		real.insert(0, msg, begin_pos + 1, std::string::npos); 
	if (real[0] == ':')
		real.erase(0, 1);
	return (real);
}

/* 
1.	checks for the client in the server using the client fd.
2.	extract the username and realname from the params.
3.	if the usrname is empty or realname is empty . send reply "ERR_NEEDMOREPARAMS".
4.	check if the client has already completed authentication. if yes send reply "ERR_ALREADYREGISTRED".
5.	if all the conditions are met set username and realname.
 */
void	user(Server *server, int client_fd, msg_struct msg_infos)
{
	Client*		client		= server->GetClient(client_fd);
	std::string	username	= findUsername(msg_infos.parameter);
	std::string realname	= findRealname(msg_infos.parameter);

	if (username.empty() || realname.empty())
		server->SendReply(client_fd, ERR_NEEDMOREPARAMS(msg_infos.cmd));
	else if (client->isAuthDone() == true)
		server->SendReply(client_fd, ERR_ALREADYREGISTRED(client->get_nickname()));
	else
	{
		client->set_username(username);
		client->set_realname(realname);
	}
}