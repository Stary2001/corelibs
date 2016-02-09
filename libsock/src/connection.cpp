#include <string>
#include <iostream>
#include <sys/types.h>

#include <fcntl.h>
#include "connection.h"
#include <errno.h>

Connection::Connection() {}

bool make_socket_non_blocking(sock_type fd)
{
	if (fd < 0) return false;

#ifdef WIN32
	unsigned long mode = 1;
	return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) return false;
	flags = flags | O_NONBLOCK;
	return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
#endif
}

Connection::Connection(int fd, void* data)
{
	m_fd = fd;
	make_socket_non_blocking(m_fd);
}

Connection::Connection(std::string host, unsigned short port)
{
    this->host = host;
    ssl = false;

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
            if(connect(m_fd, list->ai_addr, (int)list->ai_addrlen) == 0)
            {
                std::cerr << "connected to " << list->ai_family << std::endl;
                // we connected!
				freeaddrinfo(list);
                return;
            }
            else
            {
                sock_close(m_fd);
            }
        }

        list = list->ai_next;
    }

	freeaddrinfo(list);

    throw SocketException();
}

Connection::~Connection()
{
	sock_close(m_fd);
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

        int count = recv(m_fd, buf, sizeof buf, 0);
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

int Connection::read(char* buf, int len)
{
	if(ssl)
	{
		int l = gnutls_record_recv(session, buf, len);
		return l;
	}
	else
	{
		int l = recv(m_fd, buf, len, 0);
		if(l == 0)
		{
			std::cerr << "empty socket >:|" << std::endl;
			exit(0);
		}
		return l;
	}
}

int Connection::write(const char* c, size_t l)
{
    if(ssl)
    {
		gnutls_record_send(session, c, l);
	}
    else
    {
		int sent = send(m_fd, c, (int)l, 0);
		if(sent == 0)
		{
			std::cerr << "empty socket >:|" << std::endl;
			exit(0);
		}
		return sent;
	}
}

void Connection::start_ssl()
{
	gnutls_datum_t out;
    int ret;
    gnutls_certificate_type_t type;

    unsigned int status;

    gnutls_certificate_credentials_t xcred;
	gnutls_certificate_allocate_credentials(&xcred);
    gnutls_certificate_set_x509_trust_file(xcred, CAFILE, GNUTLS_X509_FMT_PEM);
    gnutls_init(&session, GNUTLS_CLIENT);
    
    gnutls_set_default_priority(session);

    gnutls_credentials_set(session, GNUTLS_CRD_CERTIFICATE, xcred);
    gnutls_session_set_verify_cert(session, host.c_str(), 0);
    gnutls_transport_set_int(session, m_fd);
    gnutls_handshake_set_timeout(session, GNUTLS_DEFAULT_HANDSHAKE_TIMEOUT);

    do 
    {
        ret = gnutls_handshake(session);
    }
    while (ret < 0 && gnutls_error_is_fatal(ret) == 0);

    if(ret < 0)
    {
        throw SocketException();
    }
    
    type = gnutls_certificate_type_get(session);
    status = gnutls_session_get_verify_cert_status(session);
    ret = gnutls_certificate_verification_status_print(status, type, &out, 0);
    if(ret < 0)
    {
        throw SocketException();
    }

    
}
