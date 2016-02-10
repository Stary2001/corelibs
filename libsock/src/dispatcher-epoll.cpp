#ifndef WIN32

#include "connection.h"
#include <iostream>

ConnectionDispatcher::ConnectionDispatcher()
{
    m_count = 0;
	m_epoll_fd = epoll_create(1);
	if (m_epoll_fd == -1)
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

	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, conn->m_fd, &ev) != 0)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}
	conn->dispatcher = this;
    m_count++;
}

void ConnectionDispatcher::remove(Connection *conn)
{
    m_count--;
	if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, conn->m_fd, NULL) != 0)
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
	if (num == -1 && errno != EINTR)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}

	for (; i < num; i++)
	{
		Connection* conn = ((Connection*)events[i].data.ptr);
		uint32_t e = 0;

		if (events[i].events & EPOLLERR)
		{
			e |= SocketEvent::Error;
		}
		else if ((events[i].events & EPOLLHUP) || (events[i].events & EPOLLRDHUP))
		{
			e |= SocketEvent::Hangup;
		}
		else if (events[i].events & EPOLLIN)
		{
			e |= SocketEvent::ReadAvail;
		}
		else if (events[i].events & EPOLLOUT)
		{
			e |= SocketEvent::WriteAvail;
		}

		conn->handle(e);
	}
}

int ConnectionDispatcher::count()
{
	return m_count;
}

#endif
