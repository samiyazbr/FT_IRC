/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ataro-ga <ataro-ga@student.42abudhabi.a    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/01/19 06:44:30 by ataro-ga          #+#    #+#             */
/*   Updated: 2024/01/19 06:44:31 by ataro-ga         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Server.hpp"
#include <cstdio>
#include <cerrno>

bool CloseServer = false; //for signal handling
void	SignalHandler(int signum)
{
    std::cout << "\r------------SIGINT CALLED-------------\n";
    (void)signum;
    CloseServer = true; 
}


Server::Server()
{
}

Server::~Server()
{
    // std::cout << "\r------------DESTRUCTOR CALLED-------------\n";
    
    close_fds();
    if (!this->client_array.empty())
    {
        std::map<int, Client *>::iterator it;
        for (it = client_array.begin(); it != client_array.end(); it++)
        {
            if (it->second)
                delete (it->second);
        }
    }

    if (!this->channel_array.empty())
    {
        std::map<std::string, Channel *>::iterator it2;
        for (it2 = channel_array.begin(); it2 != channel_array.end(); it2++)
        {
            if (it2->second)
                delete (it2->second);
        }
    }
}

Server::Server(int port, std::string pwd)
{
    this->port = port;
    this->password = pwd;
    this->server_ip = "127.0.0.1";
    this->server_name = "ft_irc";
    this->pfd_count = 0;

    time_t now = time(0); // getting current time and date
    this->creation_time = ctime(&now);

    this->CreateSocket();
    this->Listen();
    this->ConnectClients();
}

Server &Server::operator=(Server const &other)
{
    this->port = other.port;
    this->password = other.password;
    this->password = other.pfd_count;
    return (*this);
}

Server::Server(Server const &other)
{
    *this = other;
}

void Server::freeMemory()
{
    freeaddrinfo(this->servinfo);
    if (close(this->listener) == -1)
        ThrowException("FD Close Error: ");
}


void Server::CreateSocket(void)
{
    LoadAddrinfo();
    if ((this->listener = socket(this->servinfo->ai_family, this->servinfo->ai_socktype, this->servinfo->ai_protocol)) == -1)
    {
        freeaddrinfo(this->servinfo);
        ThrowException("Socket Error: ");
    }  

    // to make the port available for reuse ref:comment #1
    int optval = 1;
    if (setsockopt(this->listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)
    {
        freeMemory();
        ThrowException("SetSockOpt Error: ");
    }

    if (fcntl(this->listener, F_SETFL, O_NONBLOCK)  < 0)
    {
        freeMemory();
        ThrowException("fcntl Error: ");
    }

    if (bind(this->listener, this->servinfo->ai_addr, this->servinfo->ai_addrlen) == -1)
    {
        freeMemory();
        ThrowException("Socket Bind Error: ");
    }
    freeaddrinfo(this->servinfo);
}

std::map<std::string, Channel *> &Server::GetChannelList()
{
    return this->channel_array;
}

void Server::LoadAddrinfo(void)
{
    int status = -1;
    struct addrinfo hints;
    char port_str[15];

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;     // IPv4
    hints.ai_socktype = SOCK_STREAM; // type is stream socket
    sprintf(port_str, "%d", this->port); // convert port from int to char *
    if ((status = getaddrinfo(server_ip.c_str(), port_str, &hints, &this->servinfo)) != 0)
    {
        throw AddrInfoError(status);
    }
}

void Server::Listen(void)
{
    if (listen(this->listener, BACKLOG) == -1)
    {
        if (close(this->listener) == -1)
            ThrowException("FD Close Error: ");
        ThrowException("Socket Listen Error: ");
    }
}

void Server::ConnectClients(void)
{
    pollfd pfdStruct; // to store socket information

    // adding listener socket to list of poll fds
    pfdStruct.fd = this->listener;
    pfdStruct.events = POLLIN; // for input operations are possible on this fd
    pfdStruct.revents = 0; //setting to one to prevent conditional jump
    this->pfd_count += 1;
    this->pfds.push_back(pfdStruct);
    std::string reply;
    
    std::cout << GREEN << " *** Server running and waiting for connections *** " << RESET << std::endl;
    signal(SIGINT, SignalHandler);
    while(1)
    {
        if (CloseServer)
            break;
       int val = poll(&this->pfds[0], this->pfd_count, -1); // returns no.of elements in pollfds whose revents has been set to a nonzero value
        if (val < 0)
        {
            if (CloseServer)
                break;
            // close all fds
            close_fds();
            ThrowException("Poll Error: ");
        }
        for (int i = 0; i < this->pfd_count; i++) // going through each socket to check for incoming requests
        {
            if (pfds[i].revents & POLLIN) // checking revents if it is set to POLLIN
            {
                if (pfds[i].fd == this->listener)
                    AcceptConnections();
                else
                    ReceiveMessage(i);
            }
            if (pfds[i].revents & POLLOUT) // checking revents if it is set to POLLIN
            {
                Client *cl = this->GetClient(pfds[i].fd);
                if (cl!= NULL && !cl->messageToBeSent.empty())
                {
                    reply = cl->messageToBeSent.front();
                    if (send(pfds[i].fd, reply.c_str(), reply.length(), 0) == -1) //0 - for default behavior of send()
                        std::cout << "Unable to send data.\n";
                    cl->messageToBeSent.pop_front();
                }
            }
        }
    }
}

void Server::AcceptConnections()
{
    pollfd pfdStruct;
    int clientfd = -1;
    socklen_t addr_len;
    struct sockaddr_storage client_addr; // to store clients details
    Client *c = NULL;

    memset(&client_addr, 0, sizeof(client_addr));
    addr_len = sizeof(struct sockaddr_storage);
    clientfd = accept(this->listener, (struct sockaddr *)&client_addr, &addr_len);

    if (clientfd == -1)
        ThrowException("Accept Error: ");
    else
    {
        pfdStruct.fd = clientfd;
        pfdStruct.events = POLLIN | POLLOUT;
        pfdStruct.revents = 0;
        c = new Client(clientfd);
        client_array.insert(std::pair<int, Client *>(clientfd, c));
        this->pfds.push_back(pfdStruct);
        this->pfd_count += 1;
        std::cout << YELLOW << " *** Server got connection from " << c->get_ip_addr() << " on socket " << c->get_socket() << " *** " << RESET << std::endl;
    }
}

static void print(std::string intro, int client_fd, char *msg)
{
    if (msg != NULL)
        std::cout << std::endl
                  << intro << client_fd << " << " << BLUE << msg << RESET;
}

std::string Server::getPassword()
{
    return (this->password);
}

void Server::deleteClient(int fd)
{
    if (close(fd) == -1)
        ThrowException("FD Close Error: ");
    std::map<int, Client*>::iterator it = client_array.find(fd);
    if (it != client_array.end())
    {
        delete it->second;
        this->client_array.erase(fd);
    }
}

void Server::setPfds(int fd)
{
    for (int i = 0; i < this->pfd_count; i++)
    {
        if (pfds[i].fd == fd)
        {
            pfds[i].fd = -1;
            break;
        }
    }
}

std::string Server::getDate()
{
    return (this->creation_time);
}

void Server::ReceiveMessage(int i)
{
    char msg[4096]; // prototype says 512 char for safety...cross verify this!
    int nbytes, sender_fd;
    Client *c;

    memset(&msg, 0, sizeof(msg));
    nbytes = recv(this->pfds[i].fd, msg, sizeof(msg), 0); //0 - for default behavior of recv()
    sender_fd = pfds[i].fd;
    c = GetClient(sender_fd);
    if (nbytes <= 0)
    {
        std::cout << RED << " *** Connection Closed by Client on socket " << sender_fd << " *** \n"
                  << RESET;
        removeUserFromChannels(this, pfds[i].fd, "");
        deleteClient(pfds[i].fd);
        pfds[i].fd = -1;                // make the socket fd negative so it is ignored in future
        return;
    }
    else
    {
        // MessageStoreExecute(msg[0], pfds[i].fd);
        c->set_MsgInClient(msg);
        if (c->get_MsgFrmClient().find("\n") != std::string::npos)
        {
            const char *temp = c->get_MsgFrmClient().c_str();
			size_t found  = c->get_MsgFrmClient().find("PONG", 0);
			if (found == std::string::npos)
            	print("[Client] Message received from client ", sender_fd, (char *)temp);
            if (parseMessage(sender_fd, c->get_MsgFrmClient()) != 1)
            {
                if (c->get_MsgFrmClient().find("\n"))
                    c->get_MsgFrmClient().clear();
            }
        }
    }
}

void Server::print_messages(int fd)
{
    Client *c;
    int size;

    c = GetClient(fd);
    if (c != NULL)
    {
        size = c->message.size();
        for (int j = 0; j < size; j++)
            std::cout << c->message[j] << " ";
        std::cout << "\n---------------------\n";
    }
}

void Server::SendReply(int client_fd, std::string msg)
{
    Client *cl = this->GetClient(client_fd);
    if (cl != NULL)
      cl->messageToBeSent.push_back(msg);
}

void Server::close_fds()
{
    unsigned long i;

    for (i = 0; i < pfds.size(); i++)
    {
        if (pfds[i].fd > 0) // closed client connections are set to -1 so this check
            if (close(pfds[i].fd) == -1)
                ThrowException("FD Close Error: ");
    }
}

Client *Server::GetClient(int client_fd)
{
    std::map<int, Client *>::iterator it;
    it = this->client_array.find(client_fd);
    if (it != client_array.end())
        return (it->second);
    return (NULL);
}

Client *Server::GetClient(std::string nick)
{
    std::map<int, Client *>	client_list	= GetAllClients();
    std::map<int, Client *>::iterator it = client_list.begin();
    
    while (it != client_list.end()) //get fd of the recipient with matching nick
    {
        if (it->second->get_nickname() == nick)
        {
            return(it->second);
        }
        it++;
    }
    return (NULL);
}

Channel *Server::GetChannel(std::string name)
{
    std::map<std::string, Channel *>::iterator it;
    it = this->channel_array.find(name);
    if (it != channel_array.end())
        return (it->second);
    return (NULL);
}

std::string Server::GetServerName(void)
{
    return (this->server_name);
}

// exceptions
void Server::ThrowException(std::string err_msg)
{
    std::cout << err_msg;
    throw CustomException();
}

const char *Server::AddrInfoError::what() const throw()
{
    return (gai_strerror(this->status));
}

Server::AddrInfoError::AddrInfoError(int status) // exception constructor
{
    this->status = status;
}

const char *Server::CustomException::what() const throw()
{
    return (strerror(errno));
}

std::map<int, Client *> Server::GetAllClients()
{
    return (client_array);
}

std::string Server::getServerIP(void)
{
    return (this->server_ip);
}

void Server::RemoveChannel(std::string name)
{
    std::map<std::string, Channel *>::iterator it;
    it = this->channel_array.find(name);
    if (it != channel_array.end())
    {
        delete it->second;
        this->channel_array.erase(name);
    }
}

/* Comment #1

SO_REUSEADDR -  Allows other sockets to bind() to this port, unless there is an active
                listening socket bound to the port already. This enables you to get
                around those “Address already in use” error messages when you try to
                restart your server after a crash.

SOL_SOCKET   -  To manipulate options at the sockets API level. To manipulate options
                at any other level the protocol number of the appropriate protocol
                controlling the option is supplied. For example, to indicate that an
                option is to be interpreted by the TCP protocol, level should be set
                to the protocol number of TCP

optval       -  To access option values for setsockopt. The parameter should be non-zero
                to enable a boolean option,or zero if the option is to be disabled.

*/

/*  Why there are multiple entries in addrinfo struct?
    the network host is multihomed, accessible over multiple protocols
    (e.g., both AF_INET and AF_INET6); or the same service is available
    from multiple socket types (eg: SOCK_STREAM address or SOCK_DGRAM address).
    Therefore, the host could have multiple ways to contact it, and getaddrinfo
    is listing all known ways.
*/

/* The fcntl() file control function provides for control over open file descriptor.*/

/* Select() Poll() and Epoll()
    https://www.youtube.com/watch?v=dEHZb9JsmOU

    struct pollfd
    {
        int fd;         //file desciptor
        short events;   //requested events
        short revents;  //returned events
    };
*/

/* Message format to client

    :server 001 <nick> :Welcome to the <network> Network, <nick>[!<user>@<host>]
    <nick>: Nickname of the connecting user.
    <network>: Network name.
    <user>@<host>: Username and hostname of the connecting user.

    https://dd.ircdocs.horse/refs/numerics/001.html
    https://www.rfc-editor.org/rfc/rfc2812

*/

/*
    send() itself doesn't guarantee anything, send() only writes the data you want to send 
    over the network to the socket's buffer. There it's segmented (placed in TCP segments) 
    by the operating system, which manages the reliability of the transmission. If the 
    underlying buffer is full, then you'll get a return value that's lower that the number 
    of bytes you wanted to write. This usually indicates that the buffer is not being emptied
    by the operating system fast enough, i.e. the rate at which you write data to the buffer
    is higher than the rate at which the data is being sent to the network (or read by the 
    other party).
*/


/*
    If space is not available at the sending socket to hold the message to be transmitted, 
    and the socket file descriptor does not have O_NONBLOCK set, send() shall block until 
    space is available. If space is not available at the sending socket to hold the message 
    to be transmitted, and the socket file descriptor does have O_NONBLOCK set, send() shall 
    fail. The select() and poll() functions can be used to determine when it is possible to 
    send more data.
*/