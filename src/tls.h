#pragma once

#include "mbedtls/net_sockets.h"
#include "mbedtls/entropy.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/certs.h"
#include "socket_hal.h"

class TLS
{
public:
	int init( const char *root_crt, const size_t root_crt_len );
	int connect( const char *host, uint16_t port );
	int connect( HAL_IPAddress ip, uint16_t port );
	int write( const unsigned char *buf, size_t len );
	int read( unsigned char *buf, size_t len );
	int close();

private:
	sock_handle_t sock;
	mbedtls_entropy_context entropy;
	mbedtls_ctr_drbg_context ctr_drbg;
	mbedtls_ssl_context ssl;
	mbedtls_ssl_config conf;
	mbedtls_x509_crt cacert;

	static int send( void *ctx, const unsigned char *buf, size_t len );
	static int recv( void *ctx, unsigned char *buf, size_t len, uint32_t timeout );
	static int collect_entropy( void *data, unsigned char *output, size_t len, size_t *olen );
};
