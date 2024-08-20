/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   join.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:38:59 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/11 20:15:15 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include "Command.hpp"

/* JOIN command is used by clients to join a specific channel
	syntax: '/JOIN #channel_name [channel_key]'
	channel name: usually starts with the ('#') symbol
	channel key(optional): password for the channel if set
	Numeric Replies: ERR_BADCHANNELKEY (475)
 					ERR_CHANNELISFULL (471)
           			ERR_INVITEONLYCHAN (473)
					ERR_USERONCHANNEL (443)  				

	when the user sucessfully joins the channel the server sends replys to 
	the client and other users in the channel. the server sends a join message
	to the client indicating they have joined the channel.
	to all users that the particular user have joined the channel.
	if a topic message is set the server sends the topic to the user.
 
 */

std::vector<std::string> convert_to_vector(std::string msg);
std::vector<std::string> split(const std::string &s, char delimiter);
std::vector<std::string> splitlower(const std::string &s, char delimiter);

/* Prints out all the channels and the members & operators associated with it
1.	iterates over each channel in the channel list.
2.	for each channel its channel name is printed.
3.	followed by the members.
4.	followed by operators.
 */
void printChannelAndMembers(Server *server)
{
    std::cout << "All current channels in the server : \n";
    std::cout << "--------------------------------------\n";
    for (std::map<std::string, Channel *>::iterator it = server->GetChannelList().begin();
         it != server->GetChannelList().end(); it++)
    {
        std::cout << "Channel Name		: " << it->first << std::endl;
        std::cout << "Channel Members		: ";
        for (std::vector<Channel::Channel_Member>::iterator mem_it = it->second->members.begin();
             mem_it != it->second->members.end(); ++mem_it)
        {
            std::cout << mem_it->user->get_nickname() << ", ";
        }
        std::cout << std::endl;
        std::cout << "Channel Operators	: ";
        for (std::map<int, Client *>::iterator oper_it = it->second->operators.begin();
             oper_it != it->second->operators.end(); ++oper_it)
        {
            std::cout << oper_it->second->get_nickname() << ", ";
        }
        std::cout << std::endl;
    }
}
/* This function takes in the splitted parameters and returns a vector
of strings containing the channel keys extracted from the input params.
1. checks if there are two params in the spillted string.
2. if true, the param is splitted using the (,) delimiter and a vector of string is returned
3. if false an empty vector strings is returned.
 */
std::vector<std::string> getChannelKeys(const std::vector<std::string> &param_splitted)
{
    return param_splitted.size() == 2 ? split(param_splitted[1], ',') : std::vector<std::string>();
}

/* This function validates a list of channel names passed as input.
 */
int validateChannelNames(const std::vector<std::string> &channelNames)
{
    for (size_t i = 0; i < channelNames.size(); i++)
    {
        if (channelNames[i].size() < 2 || channelNames[i].size() > 50 || channelNames[i].find(':') != std::string::npos ||
        channelNames[i].find(7) != std::string::npos || channelNames[i].find(' ') != std::string::npos ||
        (channelNames[i][0] != '#' && channelNames[i][0] != '&' && channelNames[i][0] != '+' && channelNames[i][0] != '!'))
        {
            return i;
        }
    }
    return -1;
}

/* Handles creation of a new channel in the server. 
1.	new channel object is created and allocates memory for a new channel.
2.	 if the input channel key is not empty the input channel key is set to the newch object
	and flag is set to true.
3.	retreives the client obect associated with the client_fd from the server.
4.	this client is added as a user and operator to this channel.
5.	checks if the channel name starts with an ('!'). if true the client is 
	added as the creator of the channel.
6.	adds the channel to the servers channel list.
7.	sends message to client that says
	a.	cilent has joined the channel.
	b.	regarding the topic of the channel.
 */
void createChannel(Server *server, int client_fd, const std::string &channelName, const std::string &channelKey)
{
    Channel *newCh = new Channel(channelName);
    if (!channelKey.empty())
    {
        newCh->set_key(channelKey);
        newCh->set_key_flag(true);
    }
    Client *cl = server->GetClient(client_fd);
    newCh->addUser(cl);
    newCh->addOperator(cl);
    if (channelName[0] == '!') //only channels starting with '!' have creators
        newCh->set_creator(cl->get_nickname());
    server->GetChannelList()[channelName] = newCh;
    server->SendReply(cl->get_socket(), RPL_JOIN(cl->get_msg_prefix(), newCh->get_channel_name()));
    server->SendReply(cl->get_socket(), RPL_TOPIC(server->getServerIP(), cl->get_nickname(), newCh->get_channel_name(), newCh->get_topic()));
}

/* Getter to get the channel name 
 */
std::string Channel::get_channel_name()
{
    return (name);
}

/* adds a new user to the channel.
1.	 creates a new Channel_member object.
2.	associate the input client with the channel_member.
3.	adds this member to the list of members.
4.	updates the member count in the channel.
 */
void Channel::addUser(Client *client)
{
    Channel_Member new_member;
    new_member.user = client;
    this->members.push_back(new_member);
    this->total_members++;
}

/* Handles the process of joining the channel.
1.	checks if the channel has a key and if the channel key is the same as the one in the server
	if true a "ERR_BADCHANNELKEY" reply is sent.
2.	gets the client information associated with the input client_fd from the server.
3.	checks if the client is already a member of the channel.
	if true a "ERR_USERONCHANNEL" reply is sent.
4.	checks if the channel has a limit on the number of members and if the limit
	has been reached. if the channel is full a "ERR_CHANNELISFULL" reply is sent.
5.	cheks if the channel is an invite only channel and if the client is invited to
	the channel. if is invite only and client is not invited an "ERR_INVITEONLYCHAN"
	reply is sent.
6.	if all the checks pass the client is added to the channel.
7.	iterates over all members of the channel and sends a reply to each member that
	the client have joined the channel.
8.	sends a reply to the client regarding the topic of the channel.

 */
void joinChannel(Server *server, int client_fd, Channel *ch, const std::string &channelKey)
{
    if (ch->get_key_flag() && (ch->get_key() != channelKey || channelKey.empty()))
    {
        server->SendReply(client_fd, ERR_BADCHANNELKEY(server->GetClient(client_fd)->get_msg_prefix(), ch->get_channel_name()));
        return;
    }

    Client *cl = server->GetClient(client_fd);
    if (ch->isMember(cl->get_nickname()))
    {
        server->SendReply(client_fd, ERR_USERONCHANNEL(server->GetServerName(), cl->get_nickname(), "You are already member of that channel"));
        return;
    }

    if (ch->get_limit_flag() && ch->get_total_members() >= ch->get_limit())
    {
        server->SendReply(client_fd, ERR_CHANNELISFULL(server->GetClient(client_fd)->get_msg_prefix(), ch->get_channel_name()));
        return;
    }

    if (ch->get_invite_flag() && !(cl->isInvitedTo(ch->get_channel_name())))
    {
        server->SendReply(client_fd, ERR_INVITEONLYCHAN(server->GetClient(client_fd)->get_msg_prefix(), ch->get_channel_name()));
        return;
    }

    ch->addUser(cl);
    for (std::vector<Channel::Channel_Member>::iterator mem_it = ch->members.begin();
         mem_it != ch->members.end(); mem_it++)
    {
        server->SendReply(mem_it->user->get_socket(), RPL_JOIN(cl->get_msg_prefix(), ch->get_channel_name()));
        // mem_it->user->SendReply(mem_it->user->get_socket(), ":" + mem_it->user->get_nickname() + " JOIN " + ch->get_channel_name());
    }
    server->SendReply(cl->get_socket(), RPL_TOPIC(server->getServerIP(), cl->get_nickname(), ch->get_channel_name(), ch->get_topic()));
}

/* Takes a pointer  to a server class, an integer to the clients file descriptor 
and a struct type msg_struct as parameters. returns an int indicating success or failure.
1.	splits the parameters using convert_to_vector. the paramaters field of the msg_struct is
	split into a vector of strings. each string in the vector represents the parameter passed
	to the join command.
2.	gets the client object associated with the client_fd from server
3.	checks if the number of params is more than 2 or if the parms string is empty
	or if the param contains only #. if true an error of "ERR_NOSUCHCHANNEL" reply is send.
4.	splits first param into individual channel names using a (,) as delimiter
	and converting it into lowercase using "splitlower" function and store in a vector.
5.	extract channel keys from splitted params using the "getchannelkeys" function.
6.	check for invalid channel names using "validatechannelnames" and returns message
	"ERR_NOSUCHCHANNEL" if an invalid channel name is found.
7.	iterates over each channel name from the parameter string. for each channel name
	retrieves the channel object from the server's channel list.
	if the channel doesnot exist, create a channel using "createchannel" function.
	if the channel exists, join the channel using "joinchannel" function.
8.	after processing all channels it prints the current channels and their
	members using the "printchannelmembers" function.
9.	returns success
	*/
int join(Server *server, int client_fd, msg_struct cmd_infos)
{
    std::vector<std::string> param_splitted = convert_to_vector(cmd_infos.parameter);
    Client *cl = server->GetClient(client_fd);

    if (param_splitted.size() > 2 || cmd_infos.parameter.empty() ||
        (param_splitted.size() == 1 && param_splitted[0] == "#"))
    {
        server->SendReply(client_fd, ERR_NEEDMOREPARAMS(cmd_infos.cmd));
        return (FAILURE);
    }

    std::vector<std::string> channelNames = splitlower(param_splitted[0], ',');
    std::vector<std::string> channelKeys = getChannelKeys(param_splitted);

    int invalidChannelIndex = validateChannelNames(channelNames);
    if (invalidChannelIndex != -1)
    {
        server->SendReply(client_fd, ERR_NOSUCHCHANNEL(cl->get_nickname(), channelNames[invalidChannelIndex]));
        return (FAILURE);
    }

    for (size_t i = 0; i < channelNames.size(); i++)
    {
        std::string channelKey = i < channelKeys.size() ? channelKeys[i] : "";
        std::map<std::string, Channel *>::iterator it = server->GetChannelList().find(channelNames[i]);

        if (it == server->GetChannelList().end())
        {
            std::cout << "creating channel\n";
            createChannel(server, client_fd, channelNames[i], channelKey);
        }
        else
        {
            joinChannel(server, client_fd, it->second, channelKey);
        }
    }
    printChannelAndMembers(server);
    return (SUCCESS);
}