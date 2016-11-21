#pragma once

#include <stddef.h>
#include <stdint.h>

class TLS
{
private:
	// private stuff - no peeking!
public:
	int16_t init();
	int16_t connect(const char *host, const char *port);
	void write();
	int read(unsigned char *buf, size_t len);
	void close();
};
