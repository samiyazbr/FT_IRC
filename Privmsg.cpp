/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Privmsg.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:43:23 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/13 12:25:01 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Privmsg.hpp"

/* 
	constructor for privmsg
 */

privmsg::privmsg(Server *serv, int sender_fd, msg_struct msg_info)
{   
    this->sender_fd = sender_fd;
    this->serv = serv;
    this->params = convert_to_vector(msg_info.parameter);
    this->msg_info = &msg_info;
    CheckConditions();
}

/* 
	Handles the checkconditions function inside the constructor.

1.	check if the params vector is empty. if yes it means that there are
	no recipients for the privmsg. reply "ERR_NORECIPIENT".
2.	check if there is only one param. if yes it means only the recepients
	name was provided and there is no message. reply "ERR_NOTEXTTOSEND".
3.	checks if the first char of the first param is ('#','&','!','+'), if yes
	the message is intended to a channel and "sendtochannel" function is called.
	else "sendtoclient" function is called.
 */
void privmsg::CheckConditions()
{
    if (params.size() == 0) //if no arguments
    {
        serv->SendReply(sender_fd, ERR_NORECIPIENT(msg_info->cmd));
        return ;
    }
    else if (params.size() == 1) //if only name given but no message
    {
        serv->SendReply(sender_fd, ERR_NOTEXTTOSEND());
        return ;
    }
    if (params[0][0] == '#' || params[0][0] == '&'|| params[0][0] == '!' || params[0][0] == '+') // if msg to channel
        SendToChannel(); 
    else //if msg to user
        SendToClient();
}

/* 
	Handles the send to channel function.

1.	get the channel object corresponding to the channel name specified in the
	first param.
2.	check if the retreived channel name exist. if it dosent exist reply "ERR_NOSUCHCHANNEL".
3.	retreive the sender client object from the server as per the sender_fd.
4.	check if the sender is a member of the channel by calling ths "isMember" function of the
	channel object with the sender nickname. if the sender is not a member
	reply "ERR_CANNOTSENDTOCHANN".
5.	iterates through the members in the channel.
6.	for each member retrieves the reciver client object.
7.	check if reciever nickname is not equal to sender nickname to avoid 
	sending the message back to sender.
8.	construct the message using the "paramsjoin" function.
9.	check for leading (':') if any found remove the leading colon.
10.	send the message to the recivers socket. the message is sent to
	each member of the channel.
 */
void privmsg::SendToChannel()
{
    Client *sdr, *rcvr;
    Channel *ch = serv->GetChannel(params[0]);
    
    if (ch == NULL) //channel doesn't exist
    {
        serv->SendReply(sender_fd, ERR_NOSUCHCHANNEL(serv->GetClient(sender_fd)->get_nickname(), params[0]));
        return ;
    }
    // std::cout << "channel found....\n";
    if ((sdr = serv->GetClient(sender_fd)) != NULL)
    {
        if (ch->isMember(sdr->get_nickname()) == false) //sender not a member of channel
        {
            serv->SendReply(sender_fd, ERR_CANNOTSENDTOCHAN(params[0]));
            return ;
        }
        for(size_t i = 0; i < ch->members.size(); i++)
        {
            rcvr = ch->members[i].user;
            if (rcvr->get_nickname() != sdr->get_nickname())
            {
                std::string reply = ParamsJoin(params);
                if (reply[0] == ':')
                    reply = reply.substr(1); //removing the colon added by limechat
                serv->SendReply(rcvr->get_socket(), RPL_PRIVMSG(sdr->get_msg_prefix(), ch->get_channel_name(), reply));
            }
        }
    }
}

/* 
	Handles the send to client function.

1.	sender and reciver client objects are defined.
2.	position of ('!', '@', '%') are found and stored in variables.
3.	check if the params contains ('!'). if so extract the nickname. 
	then it means that the parameter string contains info in the 
	format "nickname!username@ipaddress".
4.	extract the username from the params string.
5.	extract the ip address from the params string.
6.	get the client associated with the nickname.
7.	check if the client exists and if his ip address or username 
	matches the extracted values. if not the reciver is set to NULL.
8.	if there is no ('!') found then check if ('%') is available. if so
	extract the nickname, hostname from the params string.
9.	get the client associated with the nickname.
10.	check if the client exists and if its username matches the extracted host.
	if not the reciver is set to null.
11. if none of these conditions are met. then the reciepient identifier
	is just a nickname.
12. if reciver is still null after all the checks it means that there is no
	user associated with the given nickname. reply send is "ERR_NOSUCHUSER".
13.	check if senders client object can be retrieved and if so construct the
	reply using "paramsjoin".
14.	check for leading (':') and if found remove it from the reply.
15. the message is send to the recivers socket.
 */
void privmsg::SendToClient()
{
    Client *sdr,*rcvr = NULL;
    std::string nick, username, ipaddr, host;
    size_t pos = params[0].find('!');
    size_t pos2 = params[0].find('@');
    size_t pos3 = params[0].find('%');
    
    if (pos != std::string::npos) //condition #1
    {
        nick = params[0].substr(0, pos);
        if (pos2 != std::string::npos)
        {
            username = params[0].substr(pos + 1, pos2 - pos - 1);
            ipaddr = params[0].substr(pos2 + 1);
            rcvr = serv->GetClient(nick);
            if (rcvr !=NULL && (rcvr->get_ip_addr() != ipaddr || rcvr->get_username() != username))
                rcvr = NULL;
        }
    }
    else if (pos3 != std::string::npos) //condition #2
    {
        nick = params[0].substr(0, pos3);
        host = params[0].substr(pos3 + 1);
        rcvr = serv->GetClient(nick);
        if (rcvr != NULL && rcvr->get_username() != host)
            rcvr = NULL;
    }
    else
        rcvr = serv->GetClient(params[0]);
        
    if (rcvr == NULL) //no user by the nick
    {
        serv->SendReply(sender_fd, ERR_NOSUCHUSER(serv->GetClient(sender_fd)->get_nickname(),params[0]));
        return ;
    }
    // std::cout << "user found....\n";
    if ((sdr = serv->GetClient(sender_fd)) != NULL)
    {   
        std::string reply = ParamsJoin(params);
        if (reply[0] == ':')
            reply = reply.substr(1); //removing the colon added by limechat
        serv->SendReply(rcvr->get_socket(), RPL_PRIVMSG( sdr->get_msg_prefix(), rcvr->get_nickname(), reply));
    }
}



