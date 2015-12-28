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
		int len = 0;
		char *last_line_end = NULL;

		while(true)
		{
			last_line_end = line_end;
			line_end = (char*)memmem(scratch + scratch_off, SCRATCH_LENGTH - scratch_off, "\r\n", 2);
			if(line_end == NULL)
			{
				if(found)
				{
					len = scratch_len - (last_line_end+2 - scratch);
					memmove(scratch, last_line_end + 2, len);
					memset(scratch + len, 0, SCRATCH_LENGTH - len);
					scratch_off = 0;
					scratch_len = len;
				}
				break;
			}

			found = true;
			len = (line_end - scratch) - scratch_off;
			std::string line(scratch + scratch_off, len);
			handle_line(line);

			len += 2;
			scratch_off += len;
		}
		
	}
}