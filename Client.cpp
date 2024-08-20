/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 07:20:04 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 07:20:07 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Client.hpp"
#include <algorithm>

Client::Client()
{
}

Client::Client(int fd)
{
    this->fd = fd;
    this->hasInfo = false;
    this->auth = false;

    struct sockaddr_in my_addr;
    socklen_t addr_len = sizeof(my_addr);
    getsockname(this->fd, (sockaddr *)&my_addr, &addr_len);
    inet_ntop(AF_INET, &my_addr.sin_addr, this->ip_addr, sizeof(ip_addr));
    this->port = ntohs(my_addr.sin_port);

	welcomeFlag = false;
	passFlag = false;
	_connection_pwd = false;
}

Client::~Client()
{
}

Client Client::operator=(Client other)
{
    this->fd = other.fd;
    return (*this);
}

char *Client::get_ip_addr()
{
    return (this->ip_addr);
}

int Client::get_port()
{
    return (this->port);
}

int Client::get_socket()
{
    return (this->fd);
}

std::string &Client::get_MsgFrmClient()
{
    return (this->_readmsg);
}

bool &Client::get_passFlag()
{
    return (this->passFlag);
}

std::string Client::get_nickname()
{
    return (this->_nickname);
}

std::string Client::get_old_nickname()
{
    return (this->_old_nickname);
}

std::string Client::get_username()
{
    return (this->_username);
}

std::string Client::get_realname()
{
    return (this->_realname);
}

bool &Client::has_all_info()
{
    return (this->hasInfo);
}

int Client::get_info()
{
    return (this->hasInfo);
}

std::string Client::get_msg_prefix()
{
    return (":" + _nickname + "!" + _username + "@" + ip_addr);
}

//////////////////////////Setters////////////////////

void Client::set_MsgInClient(std::string const &buf)
{
    this->_readmsg += buf;
}

void Client::set_nickname(std::string nick)
{
    this->_nickname = nick;
}

void Client::set_old_nickname(std::string old_nick)
{
    this->_old_nickname = old_nick;
}

void Client::set_username(std::string username)
{
    this->_username = username;
}

void Client::set_realname(std::string realname)
{
    this->_realname = realname;
}

void Client::set_passFlag(bool flag)
{
    passFlag = flag;
}

bool &Client::first_invite()
{
    return (this->welcomeFlag);
}

bool &Client::isAuthDone()
{
    return (this->auth);
}

int Client::is_valid() const
{
    if (_username.empty())
        return (FAILURE);
    if (_nickname.empty())
        return (FAILURE);
    return (SUCCESS);
}

void Client::addInvite(const std::string &channelName)
{
    invitedChannels.push_back(channelName);
}

bool Client::isInvitedTo(const std::string &channelName)
{
    return std::find(invitedChannels.begin(), invitedChannels.end(), channelName) != invitedChannels.end();
}
