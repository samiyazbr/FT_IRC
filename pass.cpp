/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pass.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:42:25 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/12 19:07:53 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Command.hpp"


/* PASS command: is used to set a ‘connection password’. 
It must match the one defined in the server.
 * 	
 * Syntax: 			PASS <password>
 * Numeric replies:	ERR_NEEDMOREPARAMS (461)
 					ERR_ALREADYREGISTRED (462)
 					ERR_PASSWDMISMATCH (464)
 */

/* Retrieves the password from the input string
1.	clears the password string.
2.	loops over the input string to skip leading white spaces
	to reach a non-space char or the end of string.
3.	iterates throught the input string adding each char to the 
	password string untill it encounters a space or reaches the end of the
	string.
4.	returns the password.
 */
std::string	retrievePassword(std::string msg_to_parse)
{
	std::string	password;
	size_t i = 0;
	
	password.clear();
	
	while (msg_to_parse[i] && msg_to_parse[i] == ' ')
		i++;
	while (msg_to_parse[i] && msg_to_parse[i] != ' ')
		password += msg_to_parse[i++];
	return (password);
}

/* 
1.	retrieves the client as per clientfd from the server.
2.	retreives the password from the input string using "retrieve password".
3.	checck if the input parameter is empty or if the password is empty.
	if true send reply "ERR_NNEDMOREPARAMS".
4.	check if provided password match with server password and if
	client hasent already been authenticated. sends reply "ERR_PASSWDMISMATCH".
5.	check if the client is already authenticated. if so send message "ERR_ALREADYREGISTERED".
6.	return success if all conditions are met.
 */
int		pass(Server *server, int client_fd, msg_struct cmd_infos)
{
	Client*	client = server->GetClient(client_fd);
	std::string	password	= retrievePassword(cmd_infos.parameter);

	if (cmd_infos.parameter.empty() == true || password.empty() == true)
	{
		server->SendReply(client_fd, ERR_NEEDMOREPARAMS(cmd_infos.cmd));
		return (FAILURE);
	}
	else if (server->getPassword() != password && client->get_passFlag() == false)
	{
		// std::cout << password << "*" << server->getPassword() << "\n";
		server->SendReply(client_fd, ERR_PASSWDMISMATCH(cmd_infos.cmd));
		password.clear();
		return (FAILURE);
	}
	else if (client->get_passFlag() == true)
		server->SendReply(client_fd, ERR_ALREADYREGISTRED(client->get_nickname()));

	else
		return (SUCCESS);
	return (SUCCESS);
}
