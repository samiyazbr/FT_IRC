/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:40:28 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/13 22:36:34 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Mode.hpp"
#include <algorithm>

/* 
	Defines the constructor

1.	assign the params to corresponding member variables of the class
2.	check the number of params is zero. if yes reply "ERR_NEEDMOREPARAMS".
2.	check if the number of params are more than 5 . if yes it means that too many
	parms were provided for the command. (there is a max limit of three changes per 
	command for modes that take a parameter.). send reply "ERR_NEEDMOREPARAMS2".
3.	finds the channel object from the first param.
4.	check if the channel exist. if the channel dosent exist reply "ERR_NOSUCHCHANNEL".
5.	check if the params size is only 1. if so it means the client wants to know the
	mode of the channel. it sends a reply to the client indicating the mode of the
	channel and returns from the funcion.
6.	gets the name of the channel and stores in a variable.
7.	if the first char of the channel name starts with ('+'), it means that
	this mode is not supported by the server. reply with "ERR_UNKNOWNMODE2".
8.	the checkmode function is called to process the mode changes.
 */

mode::mode(Server *serv, int client_fd, msg_struct msg_info)
{
    this->client_fd = client_fd;
    this->serv = serv;
    this->client = serv->GetClient(client_fd);
    this->params = convert_to_vector(msg_info.parameter);
    this->modeChanged = false;

    if (params.size() == 0)
    {
        serv->SendReply(client_fd, ERR_NEEDMOREPARAMS(msg_info.cmd));
        return;
    }

    if (params.size() > 5) // if no arguments (there is a max limit of three changes per command for modes that take a parameter.)
    {
        serv->SendReply(client_fd, ERR_NEEDMOREPARAMS2(msg_info.cmd));
        return;
    }

    this->chl = serv->GetChannel(params[0]);
    if (chl == NULL) // channel doesn't exist
    {
        serv->SendReply(client_fd, ERR_NOSUCHCHANNEL(client->get_nickname(), params[0]));
        return;
    }
    
    if (params.size() == 1) // in case of "/mode <#channel_name>"
    {
        serv->SendReply(client_fd, RPL_CHANNELMODEIS(serv->GetServerName(), client->get_nickname(), chl->get_channel_name(), chl->getChannelMode()));
        return;
    }

    std::string chlname = chl->get_channel_name();
    if(chlname[0] == '+')
    {
        //mode not supported for channels with prefix '+'
        serv->SendReply(client_fd, ERR_UNKNOWNMODE2(serv->GetServerName(), client->get_nickname()));
        return;
    }
    CheckMode();
}

/* 
	Handles processing of the mode changes for a channel and sends
	appropriate replies to clients based on the changes. checks the
	validity of the mode changes and handles associated errors.

1.	check if the current mode option is valid by searching for it
	in the array. if found the variable exists is set to true.
2.	if exists is true check if ('+') or ('-') option is provided. if yes the
	prev variable s updated accordingly.
3.	if prev variable is either ('+') or ('-') then it is updated into
	the s variable. if not ('+') is updated to the s variable.
4.	the "selectoption" function is called to set the appropriate changes.
5.	the s and the prev variables are reset.
6.	if the current mode option is invalid. reply "ERR_UNKNOWNMODE".
7.	checks if modechange occured during the above steps.
8.	if true the string with the different mode changes are constructed.
9.	retrive all the clients connected to the server in a map.
10.	iterates through the clients list and send a mode change reply to 
	each client who is a member of the channel.
 */
void mode::CheckMode()
{
    char ch, prev = '\0';
    std::vector<std::string> options;
    char arr[7] = {'i', 't', 'k', 'o', 'l', '+', '-'};
    bool exists;
    std::string s;

    // iterating through mode options
    for (std::string::size_type i = 0; i < params[1].size(); i++)
    {
        ch = params[1][i];
        exists = (std::find(arr, arr + 7, ch) != arr + 7); // checking if mode is valid
        if (exists)
        {
            if (ch == '+' || ch == '-')
                prev = ch;
            else
            {
                if (prev == '+' || prev == '-')
                    s.push_back(prev);
                else
                    s.push_back('+'); // if + or - is not given mode is +

                s.push_back(ch);
                SelectOption(s);
                s.clear();
                prev = '\0';
            }
        }
        else // invalid mode character
        {
            s.push_back(ch); // convert to string
            serv->SendReply(client_fd, ERR_UNKNOWNMODE(serv->GetServerName(), client->get_nickname(), s));
            s.clear();
        }
    }
    
    if (modeChanged)
    {
        std::string str = reply_mode + " " + reply_args;
        std::map<int, Client *> mp = serv->GetAllClients();

        for (std::map<int, Client *>::iterator it = mp.begin(); it != mp.end(); it++)
        {
            if (chl->isMember(it->second->get_nickname()))
                serv->SendReply(it->first, RPL_MODE(client->get_msg_prefix(), chl->get_channel_name(), str));
        }
    }
}

// using a hash function becauce switches dont handle strings
mode::option mode::hashit(std::string &opt)
{
    if (opt == "-i")
        return ONE;
    if (opt == "+i")
        return TWO;
    if (opt == "-t")
        return THREE;
    if (opt == "+t")
        return FOUR;
    if (opt == "-k")
        return FIVE;
    if (opt == "+k")
        return SIX;
    if (opt == "-o")
        return SEVEN;
    if (opt == "+o")
        return EIGHT;
    if (opt == "-l")
        return NINE;
    if (opt == "+l")
        return TEN;
    else
        return ZERO;
}

/* 
	Handles the modechanges. checks permissions, validates mode options
	and executes the requested mode changes accordingly. updates the reply mode string
	and sets the modechanged flag to indicate changes.


 */
void mode::SelectOption(std::string str)
{
    // proceed to excute mode only if the user has operator rights
    if (!chl->isOperator(client->get_nickname()))
    {
        serv->SendReply(client_fd, ERR_CHANOPRIVSNEEDED(serv->GetServerName(), client->get_nickname()));
        return;
    }
    switch (hashit(str))
    {
    case ZERO:
        break;

    case ONE: /* -i means remove the invite only mode */
        if (this->chl->get_invite_flag() == 1)
        {
            chl->set_invite_flag(0);
            reply_mode += "-i";
            modeChanged = true;
        }
        break;

    case TWO: /* +i means set the channel to invite only mode*/
        if (this->chl->get_invite_flag() == 0)
        {
            chl->set_invite_flag(1);
            reply_mode += "+i";
            modeChanged = true;
        }
        break;

    case THREE: /* -t removes the restrictions of the TOPIC command*/
        if (this->chl->get_topic_flag() == 1)
        {
            chl->set_topic_flag(0);
            reply_mode += "-t";
            modeChanged = true;
        }
        break;

    case FOUR: /* +t set restrictions of the TOPIC command*/
        if (this->chl->get_topic_flag() == 0)
        {
            chl->set_topic_flag(1);
            reply_mode += "+t";
            modeChanged = true;
        }
        break;

    case FIVE: /* -k remove a channel key(password)*/
        if (CheckParams('k'))
            break;
        if (this->chl->get_key_flag() == 1) // mode is changed to -k only if current flag +k
        {
            if (chl->get_key() == params[2]) // check if they key matches the current key
            {
                chl->set_key_flag(0);
                reply_mode += "-k";
                chl->set_key("");
                reply_args += (params[2] + " ");
                modeChanged = true;
            }
            else
                serv->SendReply(client_fd, ERR_KEYSET(serv->GetServerName(), client->get_nickname(), params[0]));
            params.erase(params.begin() + 2); // deleting the argument for k mode so we move to argument of next mode
        }
        break;

    case SIX: /* +k adds a channel key(password)*/
        if (CheckParams('k'))
            break;
        if (this->chl->get_key_flag() == 0) //check if there is already a key
        {
            chl->set_key(params[2]);
            chl->set_key_flag(1);
            reply_mode += "+k";
            reply_args += (params[2] + " ");
            modeChanged = true;
        }
        params.erase(params.begin() + 2); // deleting that argument
        break;

    case SEVEN: /* -o removes the operator privilages*/
        if (CheckParams('o'))
            break;
        if (chl->isMember(params[2])) // check if user is member
        {
            if (chl->isOperator(params[2])) // if an operator
            {
                chl->removeOperator(serv->GetClient(params[2]));
                reply_mode += "-o";
                reply_args += (params[2] + " ");
                modeChanged = true;
            }
        }
        else
            serv->SendReply(client_fd, ERR_USERNOTINCHANNEL(client->get_nickname(), client->get_nickname(), params[0]));
        params.erase(params.begin() + 2); // deleting that argument
        break;

    case EIGHT: /* +o adds operator privilages*/
        if (CheckParams('o'))
            break;
        if (chl->isMember(params[2])) // check if user is a member
        {
            if (!chl->isOperator(params[2])) // if not an operator already
            {
                chl->addOperator(serv->GetClient(params[2]));
                reply_mode += "+o";
                reply_args += (params[2] + " ");
                modeChanged = true;
            }
        }
        else
            serv->SendReply(client_fd, ERR_USERNOTINCHANNEL(client->get_nickname(), client->get_nickname(), params[0]));
        params.erase(params.begin() + 2); // deleting that argument
        break;

    case NINE: /* -l removes user limit to a channel*/
        if (this->chl->get_limit_flag() == 1)
        {
            chl->set_limit_flag(0);
            reply_mode += "-l";
            modeChanged = true;
        }
        break;

    case TEN: /* +l adds user limit to a channel*/
        if (CheckParams('l') == 1)
            break;
        // need not check if limit is already set
        int limit = atoi(params[2].c_str());
        if (limit < 1)
        {
            serv->SendReply(client_fd, ERR_INVALIDMODEPARAM2(serv->GetServerName(), params[0], client->get_nickname(), "l", params[2]));
            params.erase(params.begin() + 2);
            break;
        }
        chl->set_limit(limit);
        chl->set_limit_flag(1);
        reply_mode += "+l";
        reply_args += (params[2] + " ");
        params.erase(params.begin() + 2); // deleting that argument
        modeChanged = true;
        break;
    }
}

/* Handles checks for modes that needs an argument to execute. */
int mode::CheckParams(char c)
{
    if (params.size() < 3) // these mode require an argument to execute
    {
        serv->SendReply(client_fd, ERR_INVALIDMODEPARAM(serv->GetServerName(), params[0], client->get_nickname(), c));
        return (1);
    }
    return (0);
}

/*
    https://www.unrealircd.org/docs/Channel_Modes
*/
