/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   quit.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:43:46 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/12 19:49:11 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Command.hpp"
void printRcvMsg(int fd, std::string msg);

/* Quit command: is used to terminate a client’s connection to the server. 
The server acknowledges this by sending an ERROR message to the client.
 * 	
 * Syntax: 			QUIT [<Quit message>]
 * Numeric replies:	None
 */

/* 
1.	get the client details from the server using the client fd.
2.	compose the quit message.
3.	call the "removeuserfromchannels" function to remove the user from
	all the channels.
4.	send the quit message to client.
5.	prints message in the server console.
6.	remove the client from the servers client list.
7.	remove the clients fd from the set of active fds.
 */
void quit(Server *server, int fd, msg_struct cmd_infos)
{
    Client *c = server->GetClient(fd);
	std::string msg      = cmd_infos.parameter;
    std::string quit_msg = ":" + c->get_nickname() + "!" + c->get_username() + "@localhost QUIT " + msg + "\n";
    removeUserFromChannels(server, fd, quit_msg);
	
	server->SendReply(fd, RPL_QUIT(c->get_msg_prefix(), c->get_nickname(), quit_msg));
    std::cout << RED <<" *** Connection Closed by Client on socket " << fd << " *** \n" << RESET ;
    server->deleteClient(fd);
	server->setPfds(fd);
    return;
}

/* 
	Handles the process of removing the user form all the channels they were
	members of when they quit the server.

1.	retrieves the information of the client from the server based on the client_fd.
2.	retrieves the channel list into a map.
3.	iteratres over each channel in the map.
4.	check if the client is a member of the current channel.
5. 	if so, get a vector of all the members of the channel.
6.	iterates over all the members of the channel and sends a msg stating
	that the user has quit that channel.
7.	removes the user from the current channels list of members.
8.	checks if the client is the operator of the channel. then removes the operator status
	if found true.
9.	checks if the channel became empty after removal of the user and 
	checks if the channel is not a safe channel (starts with "!"), the channel
	is removed from the server.
 */
void removeUserFromChannels(Server *server, int fd, std::string msg)
{
    Client *c = server->GetClient(fd);
    std::map<std::string, Channel *> chllist = server->GetChannelList();

    for (std::map<std::string, Channel *>::iterator it = chllist.begin(); it != chllist.end(); it++ )
    {
        if (it->second->isMember(c->get_nickname()))
        {
			std::vector<struct Channel::Channel_Member> mem = it->second->getClients();
			std::vector<struct Channel::Channel_Member>::iterator mem_it = mem.begin();
			while (mem_it != mem.end())
			{
				server->SendReply(mem_it->user->get_socket() , RPL_QUIT(c->get_msg_prefix(), c->get_nickname(), msg));
				mem_it++;
			}
            it->second->removeUser(c->get_nickname());
            if (it->second->isOperator(c->get_nickname()))
                it->second->removeOperator(c);
            if (it->second->get_total_members() == 0 && it->second->get_channel_name()[0] != '!') //dont destroy safe channels starting with '!'
			{
                server->RemoveChannel(it->second->get_channel_name());
			}
        }
    }
}