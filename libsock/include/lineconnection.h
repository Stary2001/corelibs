#include "connection.h"
#include "export.h"

#define SCRATCH_LENGTH 1024

class PLUGINEXPORT LineConnection : public Connection
{
public:
	LineConnection(std::string host, unsigned short port);
	virtual ~LineConnection();
protected:
	virtual void handle(uint32_t events);
	virtual void handle_line(std::string s) = 0;

private:
	char *scratch;
	uint32_t scratch_off;
	uint32_t scratch_len;
};