/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Mode.hpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: hkunnam- <hkunnam-@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:40:39 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/02/13 21:20:00 by hkunnam-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MODE_HPP
#define MODE_HPP

#include "Replies.hpp"
#include "Channel.hpp"
#include "Server.hpp"

class Server;
class Channel;
std::vector<std::string>  convert_to_vector(std::string msg);

class mode
{
    private:
        int client_fd;
        Client *client;
        Server *serv;
        Channel *chl;
        std::vector<std::string> params; 
        std::string reply_mode; //reply sent to channel after execution
        std::string reply_args; //reply sent to channel after execution
        bool modeChanged;
        enum option{ZERO = 0, ONE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN} ;

    private:
        option hashit (std::string &opt);
        int CheckParams(char c);

    public:
        mode(Server *serv, int client_fd, msg_struct msg_info);
        void SelectOption(std::string str);
        void CheckMode();
};

#endif

/* 
	Mode command is used to set or remove options for a given target
	Command: MODE
  Parameters: <target> [<modestring> [<mode arguments>...]]
  <modestring> starts with a plus ('+', 0x2B) or minus ('-', 0x2D) character, and is made up of the following characters:

	'+': Adds the following mode(s).
	'-': Removes the following mode(s).
	'a-zA-Z': Mode letters, indicating which modes are to be added/removed.
	
Modes to be handled as per subject
· i: Set/remove Invite-only channel
· t: Set/remove the restrictions of the TOPIC command to channel operators
· k: Set/remove the channel key (password)
· o: Give/take channel operator privilege
· l: Set/remove the user limit to channel


EXAMPLE 
for command:
/mode #eight k-k+l-l+o-o+t-t+i-i 123 123 5 testuser testuser dummy

the reply sent :
<nick> has changed mode: +k-k+l-l+o-o+t-t+i-i 123 123 5 testuser testuser
*/