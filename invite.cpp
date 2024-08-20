/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   invite.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:38:00 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/12 12:50:35 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

/* INVITE command: Invite a user to a channel.
 *
 * Syntax:          INVITE <nickname> <channel>
 * 
 * Numeric Replies: ERR_NEEDMOREPARAMS (461)
 * 					ERR_NOSUCHNICK (401)
					ERR_NOSUCHCHANNEL (403)
					ERR_NOTONCHANNEL (442)
					ERR_CHANOPRIVSNEEDED (482)
					ERR_USERONCHANNEL (443)
	

 *
 */

#include "Server.hpp"
#include "Command.hpp"

std::vector<std::string> convert_to_vector(std::string msg);
std::vector<std::string> split(const std::string &s, char delimiter);

/* 
1.	splits the params and adds into vector using "convert_to_vector"
2.	retrieves the client from server using the client fd
3.	checks if the number of params is correct. if not sends a reply of "ERR_NEEDMOREPARAMS".
4.	checks if the invited user exist. if not send a reply of "ERR_NOSUCHNICK".
5.	check if the channel exist.
6.	check if user is the member of the channel. if not send a reply of "ERR_NOTONCHANNEL".
7.	check if the channel is set to invite only mode ('+i'). if so verifys that
	the client sending the invite is an operator on the channel. if not 
	sends a reply "ERR_CHANOPRIVSNEEDED".
8.	check if the invited user is already a member. if so send reply "ERR_USERONCHANNEL".
9.	check if the channel in the invite exists in the servers list of channels. if not
	return message "ERR_NOSUCHCHANNEL".
10. adds the name of the channel to the invited users invite list.
11. send the invitation message to the invited user.
12. send confirmation message to the client who invited the user.
 */
int invite(Server *server, int client_fd, msg_struct cmd_infos)
{
    std::vector<std::string> param_splitted = convert_to_vector(cmd_infos.parameter);
    Client *cl = server->GetClient(client_fd);

    // Validate the command parameters
    if (param_splitted.size() != 2)
    {
        server->SendReply(client_fd, ERR_NEEDMOREPARAMS(cmd_infos.cmd));
        return (FAILURE);
    }

    // Check if invited user exists
    Client *invitedClient = server->GetClient(param_splitted[0]);
    if (!invitedClient)
    {
        server->SendReply(client_fd, ERR_NOSUCHNICK(cl->get_nickname()));
        return (FAILURE);
    }

    // Check if channel exists
    Channel *channel = server->GetChannel(param_splitted[1]);
    if (channel)
    {
        std::string channelMode = channel->getChannelMode();
        // Check if user is a member of the channel
        if (!channel->isMember(cl->get_nickname()))
        {
            server->SendReply(client_fd, ERR_NOTONCHANNEL(param_splitted[1]));
            return (FAILURE);
        }
        if (channelMode.find('i') != std::string::npos) // checking for invite only mode
        {
            // The channel is invite-only. Ensure the user is an operator.
            if (!channel->isOperator(cl->get_nickname()))
            {
                server->SendReply(client_fd, ERR_CHANOPRIVSNEEDED(server->GetServerName(), cl->get_nickname()));
                return (FAILURE);
            }
        }

    
        // Check if invited user is already a member of the channel
        if (channel->isMember(invitedClient->get_nickname()))
        {

            server->SendReply(client_fd, ERR_USERONCHANNEL(server->GetServerName(), cl->get_nickname(), invitedClient->get_nickname() + " already member of that channel"));
            return (FAILURE);
        }
    }

    std::map<std::string, Channel *>::iterator it = server->GetChannelList().find(param_splitted[1]);

    if (it == server->GetChannelList().end())
    {
        server->SendReply(client_fd, ERR_NOSUCHCHANNEL(cl->get_nickname(), param_splitted[1]));
        return (FAILURE);
    }
    // Add the channel name to the invited user's invite list
    invitedClient->addInvite(param_splitted[1]);

    // Send the INVITE message to the invited user
    server->SendReply(invitedClient->get_socket(), RPL_INVITING(cl->get_nickname(), invitedClient->get_nickname(), param_splitted[1])); // send invite to user

    // Send a confirmation message to the user that sent the INVITE
    server->SendReply(client_fd, RPL_INVITING2(server->GetServerName(), invitedClient->get_nickname(), param_splitted[1]));

    return (SUCCESS);
}
