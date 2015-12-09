#pragma once

#include "connection.h"
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

template <class ConnType> class Listener : public Connection
{
public:
    Listener(int fd, void* data);
    Listener(std::string bind_addr, unsigned short port, void* data = NULL);
    virtual void handle(uint32_t events);
private:
    int create_listening_socket(std::string host, unsigned short port);
    void *m_data;
};

template <class ConnType> Listener<ConnType>::Listener(std::string bind, unsigned short port, void* data)
{
        m_fd = create_listening_socket(bind, port);
        m_data = data;
}

template <class ConnType> int Listener<ConnType>::create_listening_socket(std::string addr, unsigned short port)
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
        if(fd != -1)
        {
            struct sockaddr_in saddr;
                saddr.sin_family = AF_INET;
                saddr.sin_port = htons(port);

                inet_aton(addr.c_str(), &saddr.sin_addr);

                if(bind(fd, (struct sockaddr *) &saddr, sizeof(struct sockaddr_in)) != 0)
                {
                        // ono?
                        return -1;
                }
                if(listen(fd, 10) != 0)
                {
                        return -1;
                }
        }

        return fd;
}

template <class ConnType> void Listener<ConnType>::handle(uint32_t events)
{
        if(events & EPOLLIN)
	{
		sockaddr_in addr;
		socklen_t addr_len = sizeof(sockaddr_in);
		int client = accept(m_fd, (sockaddr*)&addr, &addr_len);

                /*
                
                struct sockaddr in_addr;
                  socklen_t in_len;
                char hbuf[NI_MAXHOST], sbuf[NI_MAXSERV];

                  s = getnameinfo (&in_addr, in_len,
                                   hbuf, sizeof hbuf,
                                   sbuf, sizeof sbuf,
                                   NI_NUMERICHOST | NI_NUMERICSERV);

                s = make_socket_non_blocking (infd);
                */
                

		dispatcher->add(new ConnType(client, m_data));
	}
}
