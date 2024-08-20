/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   list.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:39:28 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/11 21:36:15 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Command.hpp"
#include <cstdio>
void printRcvMsg(int fd, std::string msg);

/* list command: To list channel in the server
 * Syntax: 			LIST
 * Numeric replies:	None
 * 
 * 1. retrieves the client info from server as per the filedescriptor
 * 2. finds the list of channels from the server
 * 3. iterates through the channel list
 * 4. converts the total numbers of the channel to string and stores in the 
 * 	  members array
 * 5. construct a reply message containing info about current channel
 * 	  the number "322" is used in the resposne used for "LIST" commad in irc server.
 * 	  server name, client nickname, channel name, total members and topic
 * 6. sends the message to the client
 * 7. prints the name of the current channel to the console.
 */

void list(Server *server, int fd, msg_struct cmd_infos)
{
    Client *c = server->GetClient(fd);
	std::string msg      = cmd_infos.parameter;
    char members[20];

    std::map<std::string, Channel *>  chlist = server->GetChannelList();
    std::map<std::string, Channel *>::iterator it;

	server->SendReply(fd, RPL_MYINFO2(c->get_nickname(), server->GetServerName(), "List of Channels: "));
    std::cout << " --- channel list ---"<< std::endl;
    for (it = chlist.begin(); it != chlist.end(); it++)
    {
        sprintf(members, "%d", it->second->get_total_members()); //convert string to int
        std::string str = ":" + server->GetServerName() + " 322 " + c->get_nickname() + " " +
        it->first + " " + members + " " + it->second->get_topic() + "\r\n";	
        server->SendReply(fd, str);
        std::cout << " - " << it->first << std::endl;
    }
    return;
}