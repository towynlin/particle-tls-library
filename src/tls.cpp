#include "tls.h"

int TLS::init() {
	return 0;
}

int TLS::connect(const char *host, const char *port) {
	return mbedtls_net_connect( &server_fd, host, port, MBEDTLS_NET_PROTO_TCP );
}

int TLS::write(const char *buf) {
	return MBEDTLS_ERR_SSL_WANT_READ;
}
