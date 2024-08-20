/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   kick.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:39:16 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/11 20:10:27 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Client.hpp"
#include "Command.hpp"

std::vector<std::string> split(const std::string &s, char delimiter);
std::vector<std::string> convert_to_vector(std::string msg);
std::string ParamsJoin(std::vector<std::string> vec);


/* KICK command: is used used to request the forced removal of a user from a channel.
 * 
 * Syntax:          KICK <channel> *( "," <channel> ) <user> *( "," <user> ) [<reason>]
 * Numeric Replies: ERR_NEEDMOREPARAMS (461)
 					ERR_NOSUCHCHANNEL (403)
           			ERR_BADCHANMASK (476)    
					ERR_CHANOPRIVSNEEDED (482)
        			ERR_USERNOTINCHANNEL (441)    
					ERR_NOTONCHANNEL (442) 
	
	only channel operators can issue the kick command
	upon sending the KICK message the server processes the request and
	removes the specified user from the specified channel
	the server sends a KICK message to the user who was kicked
	the server broadcasts a KICK message to all users in the channel 
 */

/* 
	Splits the string into substrings based on a delimiter.
	stores the substrings in a vector. 
*/
std::vector<std::string>	ft_split(std::string str, char delim)
{
	std::vector<std::string> vec;
    std::string tmp = "\0";

    for (size_t i = 0; i < str.size(); i++)
    {
        if (str[i] != delim)
            tmp.push_back(str[i]);
        else
        {
            vec.push_back(tmp);
            tmp.clear();
        }
    }
    if (tmp != "\0")
        vec.push_back(tmp);
    return vec;
}

/* 
	checks if multiple nicknames are members of a channel.
	iterates through the vector of nicknames and checks each one against
	the channels member list.
	if nickname is found the function returns FAILURE else returns SUCCESS 
 */
int is_member(std::map<std::string, Channel *>::iterator it, std::vector<std::string> lst)
{
    for(size_t i = 0; i < lst.size(); i++)
    {
        if (it->second->isMember(lst.at(i)) == false)
            return (FAILURE);
    }
    return (SUCCESS);
}

/* 
	1. extracts the client from the client fd.
	2. splits the command using "convert_to_vector" function into a vector.
	3. if vector is empty send "ERR_NEEDMOREPARAMS" reply.
	4. checks if the specified channel exist.
	5. get the client issuing kick is a member and operator
		of the channel
	6. verifies that the user to be kicked is present in the channel.
	7. constructs the kick reason, considering multiple words and
		checking if a (':') needs to be added.
	8. split the list of users to be kicked based on (',') if multiple users are specified
	9. checks for errors like missing params, invalid channels, user not in channel
		insufficient privilages
	10. send "RPL_KICK" message to all users in the channel about the kick
	11. removes the kicked users from the channel and revokes any operator
		privialges.
 */
void kick(Server *server, int client_fd, msg_struct cmd_infos)
{
    Client *client					= server->GetClient(client_fd);
	std::vector<std::string> param 	= convert_to_vector(cmd_infos.parameter);//param[0]->channel & param[1]->user list & param[2]->reason
  	if (param.empty())
    {		
		server->SendReply(client_fd, ERR_NEEDMOREPARAMS(cmd_infos.cmd));
		return ;
	}
	
	std::map<std::string, Channel *>ch_lst	= server->GetChannelList();
	std::map<std::string, Channel *>::iterator it_ch = ch_lst.find(param[0]);
    std::vector<std::string> kicked_lst;
    std::vector<std::string> temp;
    std::string kicked_by = client->get_nickname();
    std::string reason = "";
    temp.push_back(kicked_by);
    
	if (param.size() > 2)
	{
		//if mutliple words are present in reason, we will need ':' at the beginning.
		//  but if there are tabs in the message ':' is not added automatically
		if (param.size() > 3 && param[2][0] != ':') 
			reason = ":";
		for (std::vector<std::string>::iterator it = param.begin() + 2; it != param.end(); it++)
			reason += (*it + " ");
	}
	else
		reason = " :byeee!";

	//check for multiple users to be kicked out
	int flag = 0;
	if (param.size() > 1)
	{
    	flag = param[1].find(',') ? 1 : 0;
		if (flag == 1)
			kicked_lst = ft_split(param[1], ',');
		else
			kicked_lst.push_back(param[1]);
	}

	//error handling
    if ((param[0].find(':') != std::string::npos) || kicked_lst.empty())
		server->SendReply(client_fd, ERR_NEEDMOREPARAMS(cmd_infos.cmd));
    else if (it_ch == ch_lst.end())
        server->SendReply(client_fd, ERR_NOSUCHCHANNEL(kicked_by, param[0]));
    else if (is_member(it_ch, kicked_lst) == FAILURE)
        server->SendReply(client_fd, ERR_USERNOTINCHANNEL(kicked_by, kicked_lst[0], param[0])); //change to kicked_lst
    else if (is_member(it_ch, temp) == FAILURE)
        server->SendReply(client_fd, ERR_NOTONCHANNEL(param[0]));
    else if (it_ch->second->isOperator(kicked_by) == false)
        server->SendReply(client_fd, ERR_CHANOPRIVSNEEDED(server->GetServerName(), kicked_by));
    else
	{
        //send rpl_kick to all channels
		std::vector<struct Channel::Channel_Member> it = it_ch->second->getClients();
        std::vector<struct Channel::Channel_Member>::iterator ite = it.begin();
        while(ite != it.end())
        {
			for (size_t i = 0; i < kicked_lst.size(); i++)
    			server->SendReply(ite->user->get_socket(), RPL_KICK(client->get_msg_prefix(), it_ch->second->get_channel_name(), kicked_lst[i], reason));
    		ite++;
        }
		for (size_t i = 0; i < kicked_lst.size(); i++)
		{
			ite = it.begin();
			it_ch->second->removeUser(kicked_lst[i]);
			while(ite != it.end())
			{
				if (ite->user->get_nickname() == kicked_lst[i])
				{
					it_ch->second->removeOperator(ite->user);
					break;
				}
				ite++;
			}
		}
	}
}