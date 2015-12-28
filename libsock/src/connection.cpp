#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netdb.h>
#include "connection.h"

#include <errno.h>
#include <unistd.h>

Connection::Connection() {}


static int make_socket_non_blocking (int sfd)
{
  int flags, s;

  flags = fcntl (sfd, F_GETFL, 0);
  if (flags == -1)
    {
      perror ("fcntl");
      return -1;
    }

  flags |= O_NONBLOCK;
  s = fcntl (sfd, F_SETFL, flags);
  if (s == -1)
    {
      perror ("fcntl");
      return -1;
    }

  return 0;
}

Connection::Connection(int fd, void* data)
{
	m_fd = fd;
	make_socket_non_blocking(m_fd);
}

/*

OutgoingConnection *OutgoingConnection::create_connection(const char* addr, int port)
{
    int status = 0;
    int fd = 0; 

    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof host_info);

    host_info.ai_family = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(addr, std::to_string(port).c_str(), &host_info, &host_info_list);
    if(status != 0) {
        return NULL; 
    }
    
    fd = socket(host_info_list->ai_family, host_info_list->ai_socktype, host_info_list->ai_protocol);
    if(fd == -1) {
        return NULL;
    }

    int reconnects = 0;   
    while(true) {
        status = connect(fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
        if(status == -1) {
            if (reconnects >= MAX_CONN_TRIES) {
                return NULL;
            }
            ++reconnects;
            continue;
        }
        break;    
    }
    OutgoingConnection *conn = new OutgoingConnection(fd);
    return conn;
}
*/


Connection::Connection(std::string host, unsigned short port)
{
    struct addrinfo hints = {0};
    struct addrinfo *list = NULL;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int r = getaddrinfo(host.c_str(), std::to_string(port).c_str(), &hints, &list);
    if(r > 0)
    {
        std::cerr << "getaddrinfo failed" << std::endl;
    }

    while(list)
    {
        m_fd = socket(list->ai_family, list->ai_socktype, list->ai_protocol);
        if(m_fd != -1)
        {
            if(connect(m_fd, list->ai_addr, list->ai_addrlen) == 0)
            {
                std::cerr << "connected to " << list->ai_family << std::endl;
                // we connected!
                return;
            }
            else
            {
                close(m_fd);
            }
        }

        list = list->ai_next;
    }

    throw SocketException();
}

Connection::~Connection()
{
	close(m_fd);
}

void Connection::handle(uint32_t events)
{
    if((events & SocketEvent::ReadAvail) == 0)
    {
        return;
    }

	bool done = false;
	while (1) // handles the simple connections
    {
        char buf[512];

        int count = ::read (m_fd, buf, sizeof buf);
        if (count == -1)
        {
			if (errno != EAGAIN)
            {
				perror ("read");
				done = true;
			}
			break;
		}
		else if (count == 0)
		{
			/* End of file. The remote has closed the connection. */
			done = true;
			break;
		}

		handle(buf, (uint32_t)count);
	}
	if(done)
	{
		delete this;
	}
}

void Connection::handle(char *buf, uint32_t len)
{
	// ... k
}

int Connection::read(void* buf, int len)
{
	int l = ::read(m_fd, buf, len);
	if(l == 0)
	{
		std::cerr << "empty socket >:|" << std::endl;
		exit(0);
	}
	return l;
}

int Connection::write(const char* c, size_t l)
{
	int sent = send(m_fd, c, l, 0);
	if(sent == 0)
	{
		std::cerr << "empty socket >:|" << std::endl;
		exit(0);
	}
	return sent;
}

ConnectionDispatcher::ConnectionDispatcher()
{
	m_epoll_fd = epoll_create(1);
	if(m_epoll_fd == -1)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}
}

void ConnectionDispatcher::add(Connection *conn)
{
	epoll_event ev;
	ev.events = EPOLLIN | EPOLLRDHUP;
	ev.data.ptr = conn;

	if(epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, conn->m_fd, &ev) != 0)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}
	conn->dispatcher = this;
}

void ConnectionDispatcher::remove(Connection *conn)
{
	if(epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, conn->m_fd, NULL) != 0)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}
	conn->dispatcher = NULL;
}

#define EPOLL_EVENTS 16
void ConnectionDispatcher::handle()
{
	epoll_event events[EPOLL_EVENTS];

	int i = 0;
	int num = epoll_wait(m_epoll_fd, events, EPOLL_EVENTS, -1);
	if(num == -1 && errno != EINTR)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}

	for(; i < num; i++)
	{
		Connection* conn = ((Connection*) events[i].data.ptr);
        uint32_t e = 0;

        if(events[i].events & EPOLLERR)
        {
            e |= SocketEvent::Error;
        }
        else if((events[i].events & EPOLLHUP) || (events[i].events & EPOLLRDHUP))
        {
            e |= SocketEvent::Hangup;
        }
        else if(events[i].events & EPOLLIN)
        {
            e |= SocketEvent::ReadAvail;
        }
        else if(events[i].events & EPOLLOUT)
        {
            e |= SocketEvent::WriteAvail;
        }

		conn->handle(e);
	}
}
