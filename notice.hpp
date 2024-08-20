/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   notice.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:41:14 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/12 22:25:17 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NOTICE_HPP
#define NOTICE_HPP

#include "Server.hpp"

std::vector<std::string>  convert_to_vector(std::string msg);
std::string ParamsJoin(std::vector<std::string> vec);

class notice
{
     private:
        int sender_fd;
        Server *serv;
        std::vector<std::string> params;
        msg_struct *msg_info;
        
    private:
        void CheckConditions();
        void SendToChannel();
        void SendToClient();
    public:
        notice(Server *serv, int sender_fd, msg_struct msg_info);
};

#endif

/* 
	  Command: NOTICE
  Parameters: <target>{,<target>} <text to be sent>
  
  notice command s used to send notices between users as well as send
  notices to channels.
  <target> is a channel name or the nickname of a client.
  <text to be sent> is the notice to be sent.
  NOTICE message is similar to PRIVMSG. the different is that automatic
  replies must never be sent in response to a NOTICE message.

 */