/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Replies.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:44:14 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 06:44:16 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REPLIES_HPP
#define REPLIES_HPP

//WELCOME MSG
#define RPL_WELCOME(servername, nickname)           (":" + servername + " 001 " + nickname + " : Welcome to the Internet Relay Network " + nickname +"\r\n")
#define RPL_YOURHOST(nickname, servername, version) (":" + servername + " 002 " + nickname + " :Your host is " + servername + ", running version " + version + "\r\n")
#define RPL_CREATED(servername, client, date)       (":" + servername + " 003 " + client + " :This server was created " + date + "\r\n")
#define RPL_MYINFO(client, servername, version, nickname, chan_modes, chan_modes_param) (":" + servername + " 004 " + client + " " + servername + " " + version + " " + nickname + " " + chan_modes + " " + chan_modes_param + "\r\n")
#define RPL_MYINFO2(client, servername, str)         (":" + servername + " 004 " + client + " " + str + "\r\n")

//NICK
#define ERR_NONICKNAMEGIVEN(nickname)			("431 " + nickname + " :No nickname given\r\n")
#define ERR_ERRONEUSNICKNAME(nickname)	        ("432 " + nickname + " :Erroneous nickname\r\n")
#define ERR_NICKNAMEINUSE(nickname)				("433 " + nickname + " :Nickname is already in use\r\n")
#define ERR_NICKCOLLISION(nickname, user, host)	("436 " + nickname + " :Nickname collision KILL from " + user + "@" + host + "\r\n")
#define ERR_UNAVAILRESOURCE(nick_channel)	    ("437 " + nick_channel + " :Nick/channel is temporarily unavailable\r\n")
#define ERR_RESTRICTED()					    ("484 :Your connection is restricted!\r\n")
#define RPL_NICK(uname, nclient, oclient)       (":" + oclient + "!" + uname + "@localhost NICK " + nclient + "\r\n") //---->check if needed

//PASS
#define ERR_NEEDMOREPARAMS(command)		("461 " + command + " :Not enough parameters\r\n")
#define ERR_ALREADYREGISTRED(client)	("462 " + client + " :Unauthorized command (already registered)\r\n")
#define ERR_PASSWDMISMATCH(client)      ("464 " + client + " :Password incorrect\r\n")

//MODE
#define ERR_NOSUCHCHANNEL(nick, channel)            ("403 " + nick + " " + channel + " :No such channel\r\n")
#define ERR_UNKNOWNMODE(server, nick, mod)          (":" + server + " 472 " + nick + " " + mod + " is not a recognised channel mode\r\n")
#define ERR_UNKNOWNMODE2(server, nick)              (":" + server + " 472 " + nick + " Changing mode is not supported for this channel\r\n")
#define ERR_KEYSET(server, nick, channel)           (":" + server + " 467 " + nick + " " + channel + " Channel key already set\r\n")
#define ERR_NOSUCHNICK(nickname)                    ("401 " + nickname +  " :No such nick/channel\r\n")
#define ERR_CHANOPRIVSNEEDED(server, nick)          (":" + server + " 482 " + nick + " :You're not channel operator\r\n")
#define ERR_NEEDMOREPARAMS2(command)		        ("461 " + command + " :Incorrect no.of Parameters\r\n")

#define ERR_UNKNOWNCOMMAND(command)	                		("421 " + command + " :Unknown command\r\n")
#define ERR_INVALIDMODEPARAM(serv, chan, nick, mod)         (":" + serv + " 696 " + nick + " " + chan + " " + mod + " * You must specify a parameter for the key mode. Syntax: <key>.\r\n")
#define ERR_INVALIDMODEPARAM2(serv, chan, nick, mod, param) (":" + serv + " 696 " + nick + " " + chan + " " + mod + " " + param + " Invalid limit mode parameter. Syntax: <limit>.\r\n")
#define RPL_CHANNELMODEIS(server, nick, channel, modes)     (":" + server + " 324 " + nick + " " + channel + " :+" + modes + "\r\n")
#define RPL_MODE(client, channel, msg)                      (client + " MODE " + channel + " " + msg + "\r\n")
#define RPL_YOUREOPER(server, nick, channel)                (":" + server + " 381 " + nick + " " + channel + ":You are now an IRC operator\r\n")

#define ERR_NORECIPIENT(cmd)                        ("411 :No recipient given " + cmd + "\r\n")
#define ERR_CANNOTSENDTOCHAN(channel)               ("404 " + channel + " :Cannot send to channel\r\n")
#define ERR_NOTEXTTOSEND()                          ("412 :No text to send\r\n")

//KICK
#define RPL_KICK(user_id, channel, kicked, reason)	(user_id + " KICK " + channel +  " " + kicked + " " + reason + "\r\n")
#define ERR_BADCHANMASK(channel)					("476 " + channel + " :Bad Channel Mask\r\n")
#define ERR_USERNOTINCHANNEL(client, nicky, channel)("441 " + client + " " + nicky + " " + channel + " :They aren't on that channel\r\n")
#define ERR_NOTONCHANNEL(channel)	 				("442 " + channel + " :You're not on that channel\r\n")

//JOIN
#define RPL_JOIN(client, channel)                   (client + " JOIN " + channel + "\r\n")
#define ERR_BADCHANNELKEY(client, channel)          ("475" + client + " " + channel + ": Cannot join channel (incorrect channel key)\r\n")
#define ERR_CHANNELISFULL(client, channel)          ("471" + client + " " + channel + ": Cannot join channel (channel is full)\r\n")
#define ERR_INVITEONLYCHAN(client, channel)         ("473" + client + " " + channel + ": Cannot join channel (+i)\r\n")

//TOPIC
#define RPL_NOTOPIC(server, nickname, channel, str)	(":" + server + " 331 " + nickname + " " + channel + " :" + str + "\r\n")
#define RPL_TOPIC(server, nickname, channel, str)	(":" + server + " 332 " + nickname + " " + channel + " :" + str + "\r\n")
#define RPL_TOPIC1(user_id, ch, str)    			(user_id + " TOPIC " + ch + " " + str + "\r\n")

//PRIVMSG
#define RPL_PRIVMSG(sender, receiver, message)     (sender + " PRIVMSG " + receiver + " :" + message +"\r\n")
#define ERR_NOSUCHUSER(nick, nick2)                ("403 " + nick + " " + nick2 + " :No such user\r\n")


//NOTICE
#define RPL_NOTICE(sender, receiver, message)     (sender + " NOTICE " + receiver + " :" + message +"\r\n")

//QUIT
#define RPL_QUIT(uname, nname, msg)     		  (uname + " QUIT " + nname + " :" + msg +"\r\n")

//PONG
#define ERR_NOORIGIN(cmd)                           ("409 " + cmd + " :No origin specified\r\n")

//INVITE
#define RPL_INVITING(nickname, recipient,channel)   (":" + nickname + " INVITE " + recipient + " " + channel + " " + nickname + "\r\n")
#define RPL_INVITING2(server, recipient,channel)    (":" + server + " 341 " + recipient + " " + recipient + " " + channel + "\r\n")
#define ERR_USERONCHANNEL(server, nickname, msg)    (":" + server + " 443 *" + nickname + " " + msg + "\r\n")


#endif  