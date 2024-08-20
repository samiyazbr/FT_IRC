/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   nick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:40:49 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/11 22:24:06 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"

/* NICK command: is used to give the client a nickname or change the previous one.
 * 
 * Syntax:          NICK <nickname>
 * Numeric Replies: ERR_NONICKNAMEGIVEN (431)
                    ERR_ERRONEUSNICKNAME (432)
                    ERR_NICKNAMEINUSE (433)
                    ERR_NICKCOLLISION (436)
                    ERR_UNAVAILRESOURCE (437)
                    ERR_RESTRICTED (484) 
 */

/* 
*	Function checks if a given nickname is valid
	1. checks the first character is found in the string of valid chars
		if found isfound is set to true else set to false.
	2. checks the first char in the nickname that is not part of the valid chars
		and stores the index of the first invalid char in the found variable.
	3. check if the nickname has spaces. i found false is returned.
	4. check if invalid chars are found or if first char is not found
		in the valid str. then false is returned.
	5. check if the length of the nickname is longer than 9 chars. return falsse if yes
	6. return true if all the conditions are met.
 */
static bool check_if_valid(std::string nick)
{
    std::size_t found;
	std::string str = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ[]\\`_- ^{}|";
  
	bool isfound = str.find(nick[0]) != std::string::npos;
    found = nick.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789[]\\`_- ^{}|");

	if (nick.find(' ') != std::string::npos)
		return (false);
    if (found != std::string::npos || !isfound)
        return (false);
    if (nick.size() > 9)
        return (false);
    return (true);
}

/* 
	Function checks if the nickname is already in use by another client 
	in the same server.

	1. retrieve a map of all clients connected to the server where the
		key is the client_fd and value is the pointer to client object.
	2. initialize and iterator and iterate through the clients in the client_list map
	3. checks if the current clients socket(fd) isnot equal to given client_fd
		and nickname of the current client is equal to the new nickname.
		return true if nickname is already used.
	4. return false if there is no client with the same nickname.
 */
bool already_used(Server *server, int client_fd, std::string new_nickname)
{
	std::map<int, Client *>	client_list	= server->GetAllClients();

	std::map<int, Client *>::iterator	it = client_list.begin();

	while (it != client_list.end())
	{
		if (it->second->get_socket() != client_fd && it->second->get_nickname() == new_nickname)
			return (true);
		it++;
	}
	return (false);
}

/* 
	1. extarct the new nickname from the msg_info.parameters and store
		it in nickname variable.
	2. retrive the client info as per fd given from the server.
	3. checks if client has completed authentication. if auth is not done
		use the "already_used" function to check if the nickname is already in use.
		if true sends a reply "ERR_NICKNAMEINUSE". if false
		checks if the name is valid using "check_if_valid" function. if it passes
		set the new nickname. if auth is done move on to next step.
	4. check if nickname is empty or only (':'). if true send "ERR_NONICKNAMEGIVEN" reply.
	5. check if nickname is valid and if invalid send "ERR_ERRONEUSNICKNAME" reply.
	6. check if nickname is already in use and if true send "ERR_ERR_NICKNAMEINUSE" reply.
	7. if everything is valid check if the client is already authenticated. then
		set old nickname to _old_nickname and set the new nickname.
	8. prints message in the console about the nickname change.
	9. sends a reply to client that confirming the nickname change and provide
		info about the old and new nickname.
 */
void nick(Server *server, int fd, msg_struct msg_info)
{
    std::string nickname = msg_info.parameter;
    Client *c = server->GetClient(fd);

    if (c->isAuthDone() == false)
	{
		if (already_used(server, fd, nickname) == true)
		{
			server->SendReply(fd, ERR_NICKNAMEINUSE(nickname));
			return ;
		}
		if (check_if_valid(nickname))
			c->set_nickname(nickname);
		else
			server->SendReply(fd, ERR_ERRONEUSNICKNAME(nickname));
	}

    if (nickname.empty() || nickname == ":")   // no nickname given
		server->SendReply(fd, ERR_NONICKNAMEGIVEN(c->get_nickname()));
    else if (check_if_valid(nickname) == false) //check valid characters and length is 8/9
		server->SendReply(fd, ERR_ERRONEUSNICKNAME(nickname));
    else if (already_used(server, fd, nickname) == true) //check if nickname given is already used by available clients
		server->SendReply(fd, ERR_NICKNAMEINUSE(nickname));
    else 
    {	
		if (c->isAuthDone() == true)
		{
            c->set_old_nickname(c->get_nickname());
			c->set_nickname(nickname);
			std::cout << "[Server] Nickname change registered. " << std::endl;
			std::cout << "Old nickname is now : " << GREEN << c->get_old_nickname() << RESET << std::endl;
			std::cout << "New nickname is now : " << GREEN << c->get_nickname() << RESET << std::endl;
    		server->SendReply(fd, RPL_NICK(c->get_username(), c->get_nickname(), c->get_old_nickname()));
		}
	}
}