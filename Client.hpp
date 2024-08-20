/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Client.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 07:20:24 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 07:20:29 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <netdb.h>	   //addrinfo
#include <arpa/inet.h> //inet_ntop
#include <vector>	   //inet_ntop
#include <deque>	

#define FAILURE -1
#define SUCCESS 0

class Client
{
private:
	int fd; // socket
	int port;
	char ip_addr[INET6_ADDRSTRLEN];
	bool auth; // set to true when authenticated by server
	bool hasInfo;
	bool welcomeFlag;
	bool passFlag;
	bool _connection_pwd;

	std::string _readmsg;
	std::string _username;
	std::string _nickname;
	std::string _realname;
	std::string _old_nickname;
	std::string _hostname;
	std::string _servername;
	std::vector<std::string> invitedChannels;
	// And add this public method:

public:
	std::string nick;
	std::vector<std::string> message;
	std::deque<std::string> messageToBeSent;
	// std::vector <std::vector<std::string> > message;

public:
	Client();
	Client(int fd);
	~Client();
	Client operator=(Client other);

	char *get_ip_addr();
	int get_port();
	int get_socket();
	std::string &get_MsgFrmClient();
	int get_info();
	std::string get_nickname();
	std::string get_old_nickname();
	std::string get_username();
	std::string get_realname();
	std::string get_msg_prefix();
	bool &isAuthDone();
	bool &has_all_info();
	bool &get_passFlag();
	bool &first_invite();

	bool isInvitedTo(const std::string &channelName);

	void addInvite(const std::string &channelName);

	void set_MsgInClient(std::string const &buf);
	void set_nickname(std::string nick);
	void set_old_nickname(std::string old_nick);
	void set_username(std::string username);
	void set_realname(std::string realname);
	void set_passFlag(bool flag);
	int is_valid() const;
};

#endif