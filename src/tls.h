#pragma once

#include <net_sockets.h>

class TLS
{
public:
	int init();
	int connect(const char *host, const char *port);
	int write(const char *buf);
	int read(unsigned char *buf, size_t len);
	void close();

private:
	mbedtls_net_context server_fd;
};
