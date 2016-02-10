#ifdef WIN32

#include "connection.h"
#include <iostream>

ConnectionDispatcher::ConnectionDispatcher()
{
    m_count = 0;
}

void ConnectionDispatcher::add(Connection *conn)
{
	m_fds.push_back({ conn->m_fd, POLLRDNORM | POLLWRNORM, 0 });
	m_conns[conn->m_fd] = conn;
	conn->dispatcher = this;
    m_count++;
}

void ConnectionDispatcher::remove(Connection *conn)
{
	auto it = m_fds.begin();
	for (; it != m_fds.end(); it++)
	{
		if (it->fd == conn->m_fd)
		{
			break;
		}
	}

	if (it != m_fds.end())
	{
		m_fds.erase(it);
	}

	m_conns.erase(conn->m_fd);

	conn->dispatcher = NULL;
    m_count++;
}

void ConnectionDispatcher::handle()
{
	int i = 0;
	int num = WSAPoll(&m_fds[0], m_fds.size(), 0);

	if (num == -1 && errno != EINTR)
	{
		std::cerr << "errno " << errno << std::endl;
		throw EpollException();
	}

	auto it = m_fds.begin();

	for (; it != m_fds.end(); it++)
	{
		Connection* conn = m_conns[it->fd];
		uint32_t e = 0;

		if (it->revents & POLLERR)
		{
			e |= SocketEvent::Error;
		}
		else if (it->revents & POLLHUP)
		{
			e |= SocketEvent::Hangup;
		}
		else if (it->revents & POLLRDNORM)
		{
			e |= SocketEvent::ReadAvail;
		}
		else if (it->revents & POLLWRNORM)
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
