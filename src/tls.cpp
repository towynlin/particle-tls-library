#include "tls.h"

int16_t TLS::init() {
	return 0;
}

int16_t TLS::connect(const char *host, const char *port) {
	return mbedtls_net_connect( &server_fd, host, port, MBEDTLS_NET_PROTO_TCP );
}
