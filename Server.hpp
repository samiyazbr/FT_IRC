/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:44:44 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 06:44:46 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h> //addrinfo
#include <exception>
#include <cstring> //c_str()
#include <iostream>
#include <unistd.h> //close
#include <stdlib.h> //malloc
#include <poll.h>
#include <fcntl.h>
#include <arpa/inet.h> //inet_ntop
#include <vector>
#include <map>
#include <ctime>
#include <csignal> 
#include <string>
#include "Client.hpp"
#include "Replies.hpp"
#include "Command.hpp"
#include "Mode.hpp"
#include "Privmsg.hpp"
#include "notice.hpp"

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"
#define YELLOW "\033[33m"
#define BLUE "\033[34m"
#define BACKLOG 10 // no.of connections in incoming queue that waits till accept()

class Channel;
class Server
{
private:
    int port;
    int listener;
    std::string password;
    std::string server_ip;
    std::string server_name;
    std::string creation_time;
    struct addrinfo *servinfo;
    std::vector<pollfd> pfds; // vector to store poll fds
    int pfd_count;            // to store file descriptors to be polled
    std::map<int, Client *> client_array;
    std::map<std::string, Channel *> channel_array;
    // Command *cmd_cls;

public:
    Server();
    Server(int port, std::string pwd);
    Server(Server const &other);
    Server &operator=(Server const &other);
    ~Server();
    std::string GetServerName(void);
    Client *GetClient(int client_fd);
    Client *GetClient(std::string nick);
    Channel *GetChannel(std::string name);
    void SendReply(int client_fd, std::string msg);
    std::map<int, Client *> GetAllClients();
    std::string getPassword();
    void deleteClient(int fd);
    void setPfds(int fd);
    std::string getDate();
    std::map<std::string, Channel *> &GetChannelList();
    std::string getServerIP(void);
    void RemoveChannel(std::string name);

private:
    void CreateSocket(void);
    void LoadAddrinfo(void);
    void Listen(void);
    void ConnectClients(void);
    void AcceptConnections();
    void ThrowException(std::string err_msg);
    void ReceiveMessage(int i);
    void MessageStoreExecute(char c, int client_fd);
    void print_messages(int fd);
    void close_fds();
    int  parseMessage(int fd, std::string msg);
    void fillDetails(Client *c, int client_fd, std::string cmd);
    int  execCommand(int const client_fd, std::string cmd_line);
    void freeMemory();

    class AddrInfoError : public std::exception // custom exception
    {
    private:
        int status;

    public:
        const char *what() const throw();
        AddrInfoError(int status); // constructor
    };
    class CustomException : public std::exception // custom exception
    {
    public:
        const char *what() const throw();
    };
};

void removeUserFromChannels(Server *server, int fd, std::string str);


#endif