/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Channel.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 07:19:40 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 07:19:42 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <vector>
#include <iostream>
#include "Client.hpp"
#include "Server.hpp"

class Server;

class Channel
{

private:
        std::string name; // channel name
        std::string topic;
        std::string key; // password
        std::string creator; // channel name
        int limit;
        int total_members; // current number of members present in the channel
        // modes
        bool iflag; // invite_only flag --> +i = 1, -i = 0
        bool tflag; // topic flag
        bool kflag; // key or password flag
        bool lflag; // flag to set limit

public:
        struct Channel_Member
        {
                Client *user;
        };
        std::map<int, Client *> operators;   // to store client_fds of operators
        std::vector<Channel_Member> members; // store all the members in the channel
        // default constructor
        Channel();

        // construct with channel name
        Channel(std::string name);
        void removeUser(const std::string &nickname);

        // getters and setters
        bool get_invite_flag();
        void set_invite_flag(bool i);
        bool get_topic_flag();
        void set_topic_flag(bool i);
        bool get_key_flag();
        void set_key_flag(bool i);
        bool get_limit_flag();
        void set_limit_flag(bool i);
        std::string get_topic();
        void set_topic(std::string str);
        int get_total_members();
        std::string get_channel_name();
        std::string get_key();
        void set_key(std::string key);
        int get_limit();
        void set_limit(int val);
        std::string get_creator();
        void set_creator(std::string str);

        // channel join validation
        bool isMember(std::string nick);
        bool isOperator(std::string nick);
        bool isFull();

        // channel actions
        void addUser(Client *client);
        void addOperator(Client *client);
        void removeOperator(Client *client);
        std::string getChannelMode();
        std::vector<Channel_Member> getClients();
        // void createChannel(Server *server, int client_fd, const std::string &channelName,
        //                    const std::string &channelKey);

        // PRINT ALL CHANNEL AND MEMBERS IN THE SERVER
        // void printChannelAndMembers(Server *server);

        // std::vector<std::string> getChannelKeys(const std::vector<std::string>
        //                                             &param_splitted);
        // int validateChannelNames(const std::vector<std::string> &channelNames);
        // void joinChannel(Server *server, int client_fd, Channel *ch, const std::string &channelKey);
        // int join(Server *server, int client_fd, msg_struct cmd_infos);
};

#endif

/* modes explained

    invite  - set to 0 initially, which means anyone can join (-i)
            - can be changed to 1 to make channel invite only (+i)

    topic   - set to 1 initially, which means only operators can change topic (+t)
            - can be changed to 0 to let anybody change the topic (-t)

    key     - set to 0 initially, which means no pwd needed to join channel (-k)
            - can be changed to 1 to set password (+k)

    limit   - set to 0 initially, which means no limit on no.of users(-l)
            - can be changed to 1 to set user limit (+l)
*/

// https://github.com/irssi/irssi/blob/master/docs/manual.txt