#pragma once

#include <stddef.h>
#include <stdint.h>
#include <net_sockets.h>

class TLS
{
public:
	int16_t init();
	int16_t connect(const char *host, const char *port);
	int16_t write(const char *buf);
	int read(unsigned char *buf, size_t len);
	void close();

private:
	mbedtls_net_context server_fd;
};
