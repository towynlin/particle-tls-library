#pragma once

#include <stddef.h>
#include <stdint.h>

class TLS
{
private:
	// private stuff - no peeking!
public:
	int16_t init();
	void connect();
	void write();
	int read(unsigned char *buf, size_t len);
	void close();
};
