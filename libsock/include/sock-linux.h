#include <unistd.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <sys/socket.h>

#define sock_type int
#define sock_close ::close