#pragma once

#include <net_sockets.h>
#include <entropy.h>
#include <ctr_drbg.h>
#include <certs.h>

class TLS
{
public:
	int init( const char *root_pem );
	int connect( const char *host, const char *port );
	int write( const unsigned char *buf, size_t len );
	int read( unsigned char *buf, size_t len );
	void close();

private:
	mbedtls_net_context server_fd;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cacert;
};
