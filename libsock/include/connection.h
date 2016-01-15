#pragma once
#include <string>
#include <stdint.h>
#include <vector>
#include <map>
#include "socketevent.h"

#ifdef WIN32
#include "sock-windows.h"
#else
#include "sock-linux.h"
#endif

class ConnectionDispatcher;
class Connection
{
	friend class ConnectionDispatcher;
public:
	Connection();
	~Connection();
	Connection(int fd, void* data);
	Connection(std::string host, unsigned short port);
	virtual void handle(uint32_t events); //
	virtual void handle(char *buf, uint32_t len);
	int read(char* buf, int len);
	int write(const char* , size_t);
	
	sock_type m_fd;
protected:
	ConnectionDispatcher *dispatcher;
};

class ConnectionDispatcher
{
public:
        ConnectionDispatcher();
        void add(Connection *conn);
        void remove(Connection *conn);
        void handle();
private:
        int m_epoll_fd;
#ifdef WIN32
		std::vector<WSAPOLLFD> m_fds;
		std::map < sock_type, Connection*> m_conns;
#endif
		
};


class EpollException : std::exception
{};

class SocketException : std::exception
{};
