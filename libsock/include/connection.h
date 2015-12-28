#pragma once
#include <string>
#include <stdint.h>
#include "socketevent.h"

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
	int read(void* buf, int len);
	int write(const char* , size_t);
	
	int m_fd;
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
};


class EpollException : std::exception
{};

class SocketException : std::exception
{};
