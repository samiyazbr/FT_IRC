/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   topic.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:44:55 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/12 21:13:13 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Command.hpp"

std::vector<std::string> convert_to_vector(std::string msg);
std::vector<std::string> split(const std::string &s, char delimiter);
void set_topic(std::vector<std::string> vec, Channel *ch, Client *cl, Server *s);

/*  Command: TOPIC
  Parameters: <channel> [<topic>]
  command is used to change or view the topic of the given channel.
  Numeric Replies:	ERR_NEEDMOREPARAMS (461)
					ERR_NOSUCHCHANNEL (403)
					ERR_NOTONCHANNEL (442)
					ERR_CHANOPRIVSNEEDED (482)
					RPL_NOTOPIC (331)
					RPL_TOPIC (332)
   */
/* 
1.	splits the params and adds into a vector.
2.	retrieves the client associated with the client fd.
3.	checks for the number of params is atleast 1(channel name). if not reply "ERR_NEEDMOREPARAMS".
4.	checks for the channel name in the list of channels. if not found reply "ERR_NOSUCHCHANNEL".
5.	checks if the client is a member of  the channel. if not reply "ERR_NOTONCHANNEL". 
6.	check if the topic command has atleast 2 params. means client wants to change the topic.
	if so check if channel has the topic flag set and if the client is not an operator.
	if so send reply "ERR_CHANOPRIVSNEEDED" that operator privilages are needed.
	if everything checksout then "set_topic" is called to set the topic.
7.	else the client wants to know the current topic. check if the topic is empty
	if so send no topic set message. if not empty send the current topic.

 */
int topic(Server *server, int client_fd, msg_struct cmd_infos)
{
	std::string str;
    std::vector<std::string> param_splitted = convert_to_vector(cmd_infos.parameter);
    Client *cl = server->GetClient(client_fd);

    if (param_splitted.size() < 1)
    {
        server->SendReply(client_fd, ERR_NEEDMOREPARAMS(cmd_infos.cmd));
        return (FAILURE);
    }

    std::string channelName = param_splitted[0];
    std::map<std::string, Channel *>::iterator it = server->GetChannelList().find(channelName);
    if (it == server->GetChannelList().end())
    {
        server->SendReply(client_fd, ERR_NOSUCHCHANNEL(cl->get_nickname(), channelName));
        return (FAILURE);
    }

    Channel *ch = it->second;
    if (!ch->isMember(cl->get_nickname()))
    {
        server->SendReply(client_fd, ERR_NOTONCHANNEL(ch->get_channel_name()));
        return (FAILURE);
    }

    // If 2 parameters are given, the client wants to change the topic
    if (param_splitted.size() >= 2)
    {
        // Check for the operator privileges only if channel mode +t, if -t no need to check operator privileges
        if (ch->get_topic_flag() && !ch->isOperator(cl->get_nickname()))
        {
            server->SendReply(client_fd, ERR_CHANOPRIVSNEEDED(server->getServerIP(), cl->get_nickname()));
            return (FAILURE);
        }

        // Set the new topic
		set_topic(param_splitted, ch, cl, server);
    }
    else // The client wants to know the current topic
    {
        if (ch->get_topic().empty())
        {
            std::cout << "Topic of the channel : "  << GREEN << "No topic set" << RESET << "\n";
            server->SendReply(client_fd, RPL_NOTOPIC(server->GetServerName(), cl->get_nickname(), ch->get_channel_name(), "no topic set"));
        }
        else
        {
            std::cout << "Topic of the channel : "  << GREEN << ch->get_topic() << RESET << "\n";
            server->SendReply(client_fd, RPL_TOPIC(server->GetServerName(), cl->get_nickname(), ch->get_channel_name(), ch->get_topic()));
        }
    }
    return (SUCCESS);
}

/* 
	Handles setting the topic for the current channel.

1.	check if the params vector contains exactly two elements means	
	client wants to set a new topic.
2.	check if the first char of the second param is (':'),
	means client wants to clear the topic. if the condition is met clear the 
	topic.
3.	if the (':') is not found at the begining of the second param, set
	the topic to the second param.
4.	get the list of channel members and iterate through all the channel members.
	send reply containing the new topic to each channel member.
5. if the params vector contains more than two vectors means the client
	wants to set a new topic with additional text.
6.	check if the first char of the second param is (':') means the 
	client wants to set a new topic with additional text.
7.	join all the remaining params to make the topic string.
8.	set the topic using this string.
9.	get an iterator pointing to a vector containing channel members.
10.	loop through channel members and send a reply containing the new
	topic to each channel member.
	
 */
void set_topic(std::vector<std::string> param_splitted, Channel *ch, Client *cl, Server *server)
{
	std::string str;

	if (param_splitted.size() == 2)
	{
		if (param_splitted[1][0] == ':') //clear topic when only ":" is passed
			ch->set_topic("");
		else
			ch->set_topic(param_splitted[1]);
		std::vector<struct Channel::Channel_Member> it = ch->getClients();
       	std::vector<struct Channel::Channel_Member>::iterator ite = it.begin();
       	while(ite != it.end())
       	{
    		server->SendReply(ite->user->get_socket(), RPL_TOPIC1(cl->get_msg_prefix(), ch->get_channel_name(), ch->get_topic()));
    		ite++;
       	}
	}
	else
	{
		if (param_splitted[1][0] == ':')
		{
			str = param_splitted[1].substr(1) + " ";
			for (size_t i = 2; i < param_splitted.size(); i++)
				str += param_splitted[i] + " ";
		}
		ch->set_topic(str);
		std::vector<struct Channel::Channel_Member> it = ch->getClients();
       	std::vector<struct Channel::Channel_Member>::iterator ite = it.begin();
       	while(ite != it.end())
       	{
    		server->SendReply(ite->user->get_socket(), RPL_TOPIC1(cl->get_msg_prefix(), ch->get_channel_name(), ":" + ch->get_topic()));
    		ite++;
       	}
		str.clear();
	}
}
