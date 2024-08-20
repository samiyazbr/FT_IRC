/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 07:19:20 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 07:19:28 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Channel.hpp"
// #include "Client.hpp"

Channel::Channel()
{
}

void Channel::removeUser(const std::string &nickname)
{
    std::vector<Channel_Member>::iterator it;
    for (it = members.begin(); it != members.end(); ++it)
    {
        if (it->user->get_nickname() == nickname)
        {
            members.erase(it);
            total_members--;
            break;
        }
    }
}

Channel::Channel(std::string name)
{
    this->name = name;
    iflag = 0;
    tflag = 1;
    kflag = 0;
    lflag = 0;
    this->total_members = 0;
    this->topic = "";
    this->key = "";
    this->creator = "";
}

bool Channel::isMember(std::string nick)
{
    for (long unsigned int i = 0; i < this->members.size(); i++)
    {
        if (this->members[i].user->get_nickname() == nick)
            return (true);
    }
    return (false);
}

bool Channel::isOperator(std::string nick)
{
    std::map<int, Client *>::iterator it = operators.begin();

    while (it != operators.end())
    {
        if (it->second->get_nickname() == nick)
            return (true);
        it++;
    }
    return (false);
}

bool Channel::isFull()
{
    return this->total_members >= limit;
}

int Channel::get_total_members()
{
    return (total_members);
}

bool Channel::get_invite_flag()
{
    return iflag;
}

void Channel::set_invite_flag(bool i)
{
    iflag = i;
}

bool Channel::get_topic_flag()
{
    return tflag;
}

void Channel::set_topic_flag(bool i)
{
    tflag = i;
}

bool Channel::get_key_flag()
{
    return kflag;
}

void Channel::set_key_flag(bool i)
{
    kflag = i;
}

bool Channel::get_limit_flag()
{
    return lflag;
}

void Channel::set_limit_flag(bool i)
{
    lflag = i;
}

std::string Channel::get_key()
{
    return this->key;
}

void Channel::set_key(std::string key)
{
    this->key = key;
}

int Channel::get_limit()
{
    return this->limit;
}

void Channel::set_limit(int val)
{
    this->limit = val;
}

std::string Channel::get_topic()
{
    return this->topic;
}

void Channel::set_topic(std::string str)
{
    this->topic = str;
}

std::string Channel::get_creator()
{
    return this->creator;
}

void Channel::set_creator(std::string str)
{
    this->creator = str;
}

void Channel::addOperator(Client *client)
{
    int client_fd = client->get_socket();
    this->operators.insert(std::pair<int, Client *>(client_fd, client));
}

void Channel::removeOperator(Client *client)
{
    int client_fd = client->get_socket();
    this->operators.erase(client_fd);
}

std::string Channel::getChannelMode()
{
    std::string str;
    if (iflag == 1)
        str += "i";
    if (tflag == 1)
        str += "t";
    if (kflag == 1)
        str += "k";
    if (lflag == 1)
        str += "l";
    return (str);
}

std::vector<struct Channel::Channel_Member> Channel::getClients()
{
    return (this->members);
}
