#include <iostream>
#include <string.h>
#include "lineconnection.h"

LineConnection::LineConnection(std::string host, unsigned short port) : Connection(host, port)
{
	scratch = new char[SCRATCH_LENGTH];
	scratch_off = 0;
	scratch_len = 0;
}

LineConnection::~LineConnection() 
{
	delete[] scratch;
}

#ifdef WIN32
const void* memmem(const void *l, size_t l_len, const void *s, size_t s_len);
#endif

void LineConnection::handle(uint32_t events)
{
	if(events & SocketEvent::Hangup) // if server disconnected, die. f.
	{
		dispatcher->remove(this);
		delete this;
		return;
	}

	if(events & SocketEvent::ReadAvail)
	{
		int r = read(scratch + scratch_len, SCRATCH_LENGTH - scratch_len);
		if(r == 0)
		{
			// ??
			std::cerr << "read returned 0?" << std::endl;
		}
		if(r == -1)
		{
			std::cerr << errno << std::endl;
		}
		scratch_len += r;

		char *line_end = NULL;
		bool found = false;
		uint32_t len = 0;
		char *last_line_end = NULL;

		while(true)
		{
			last_line_end = line_end;
			line_end = (char*)memmem(scratch + scratch_off, SCRATCH_LENGTH - scratch_off, "\r\n", 2);
			if(line_end == NULL)
			{
				if(found)
				{
					len = scratch_len - (uint32_t)(last_line_end+2 - scratch);
					memmove(scratch, last_line_end + 2, len);
					memset(scratch + len, 0, SCRATCH_LENGTH - len);
					scratch_off = 0;
					scratch_len = len;
				}
				break;
			}

			found = true;
			len = (uint32_t)(line_end - scratch) - scratch_off;
			std::string line(scratch + scratch_off, len);
			handle_line(line);

			len += 2;
			scratch_off += len;
		}
		
	}
}

#ifdef WIN32
const void * memmem(const void *l, size_t l_len, const void *s, size_t s_len)
{
	register char *cur, *last;
	const char *cl = (const char *)l;
	const char *cs = (const char *)s;

	/* we need something to compare */
	if (l_len == 0 || s_len == 0)
		return NULL;

	/* "s" must be smaller or equal to "l" */
	if (l_len < s_len)
		return NULL;

	/* special case where s_len == 1 */
	if (s_len == 1)
		return memchr(l, (int)*cs, l_len);

	/* the last position where its possible to find "s" in "l" */
	last = (char *)cl + l_len - s_len;

	for (cur = (char *)cl; cur <= last; cur++)
		if (cur[0] == cs[0] && memcmp(cur, cs, s_len) == 0)
			return cur;

	return NULL;
}
#endif