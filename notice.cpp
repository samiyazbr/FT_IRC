/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   notice.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:41:02 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/13 11:26:11 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "notice.hpp"

/* 
	Constructor for the notice class.
 */
notice::notice(Server *serv, int sender_fd, msg_struct msg_info)
{   
    this->sender_fd = sender_fd;
    this->serv = serv;
    this->params = convert_to_vector(msg_info.parameter);
    this->msg_info = &msg_info;
    CheckConditions();
}

/* 
	Handles the checkconditions function.

1.	checks if the params vector is empty.if yes means that there are 
	no recepients specified for the notice message, reply "ERR_NORECEPIENT".
2.	checks if there is only one params. if yes means that 
	only the recepients name was provided, but no message was included 
	reply "ERR_NOTEXTTOSEND".
3.	checks if the first char of the parms is ('#', '&', '!', '+'), if so
	the notice message is intended for a channel. so the "sendToChannel" function
	is called. else the message is for a client and "sendToClient" function is called.

 */

void notice::CheckConditions()
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
	Handles the SendToChannel function.

1.	get the channel object corresponding to the channel name specified
	in the first param.
2.	check if the retreived channel object is NULL. if yes the channel 
	dosent exist. send reply "ERR_NOSUCHCHANNEL".
3.	retreive the sender client object from the server as per the fd.
4.	check if the sender is a member of the channel by calling the "isMember" function.
	of the channel object with the sender nickname. if not a member send reply "ERR_CANNOTSENDTOCHAN".
5.	iterates over the members in the channel. 
6.	for each member retreives the client object and assign to 'rcvr'.
7.	check if the receivers nickname is different from the senders nickname
	to avoid sending the notice to the sender.
8.	construct reply using paramsjoin. removes the leading colon from the message
	if any.
9.	sends the notice message to the receivers scocket. the message is send to each
	member of the channel.
 */
void notice::SendToChannel()
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
                serv->SendReply(rcvr->get_socket(), RPL_NOTICE(sdr->get_msg_prefix(), ch->get_channel_name(), reply));
            }
        }
    }
}

/* 
	Handles the sendtoclient function.

1.	initialize pointers to client objects for sender and receiver.
2.	retrieves the client object for the reciver using the nickname
	specified in the first param of the params vector.
3.	check if the retrieved client object is NULL. if so reply "ERR_NOSUCHCHANNEL".
4.	retrive the client object corresponding to the senders fd.
5.	construct the message using "paramsjoin" to join the messages into a single string.
6.	check for leading (':') and if found removes it.
7.	send the notice message to the recivers socket.
 */
void notice::SendToClient()
{
    Client *sdr,*rcvr = NULL;
    
    rcvr = serv->GetClient(params[0]);
        
    if (rcvr == NULL) //no user by the nick
    {
        serv->SendReply(sender_fd, ERR_NOSUCHCHANNEL(serv->GetClient(sender_fd)->get_nickname(),params[0]));
        return ;
    }
    // std::cout << "user found....\n";
    if ((sdr = serv->GetClient(sender_fd)) != NULL)
    {   
        std::string reply = ParamsJoin(params);
        if (reply[0] == ':')
            reply = reply.substr(1); //removing the colon added by limechat
        serv->SendReply(rcvr->get_socket(), RPL_NOTICE( sdr->get_msg_prefix(), rcvr->get_nickname(), reply));
    }
}

