#include "tls.h"
#include "rng_hal.h"
#include <cstring>
#include <cstddef>

/* debug code start */
#include "spark_wiring_usbserial.h"

static void my_debug( void *ctx, int level,
                      const char *file, int line,
                      const char *str )
{
	Serial.printlnf( "%s:%04d: %s", file, line, str );
}
/* debug code end, other than the call to mbedtls_ssl_conf_dbg below */


int TLS::init( const char *root_crt, const size_t root_crt_len )
{
	int error;

	mbedtls_ssl_init( &ssl );
	mbedtls_ssl_config_init( &conf );
	mbedtls_x509_crt_init( &cacert );
	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_entropy_init( &entropy );

	mbedtls_entropy_add_source( &entropy, collect_entropy, nullptr,
	                            MBEDTLS_ENTROPY_BLOCK_SIZE,
	                            MBEDTLS_ENTROPY_SOURCE_STRONG );

	const char *personalization = "particle-tls-library";
	error = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
	                               (const unsigned char *)personalization,
	                               strlen( personalization ));
	if( error < 0 )
	{
		return error;
	}

	error = mbedtls_x509_crt_parse( &cacert,
	                                (const unsigned char *) root_crt,
	                                root_crt_len );

	if ( error < 0 )
	{
		return error;
	}

	mbedtls_ssl_set_bio( &ssl,
	                     this,
	                     &TLS::send,
	                     nullptr,
	                     &TLS::recv );

	error = mbedtls_ssl_config_defaults( &conf,
	                                     MBEDTLS_SSL_IS_CLIENT,
	                                     MBEDTLS_SSL_TRANSPORT_STREAM,
	                                     MBEDTLS_SSL_PRESET_DEFAULT );

	if( error != 0 )
	{
		return error;
	}

	mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_REQUIRED );
	mbedtls_ssl_conf_ca_chain( &conf, &cacert, nullptr );
	mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );
	mbedtls_ssl_conf_dbg( &conf, my_debug, nullptr );

	return mbedtls_ssl_setup( &ssl, &conf );
}

int TLS::connect( const char *host, uint16_t port )
{
	HAL_IPAddress ip;
	int rv = inet_gethostbyname( host, strlen(host), &ip, 0, nullptr );
	if( rv != 0 )
	{
		return MBEDTLS_ERR_NET_UNKNOWN_HOST;
	}

	int error = mbedtls_ssl_set_hostname( &ssl, host );

	if( error != 0 )
	{
		return error;
	}

	return connect( ip, port );
}

int TLS::connect( HAL_IPAddress ip, uint16_t port )
{
	this->sock = socket_create(AF_INET, SOCK_STREAM, IPPROTO_TCP, port, 0);

	sockaddr_t sa;
	sa.sa_family = AF_INET;
	sa.sa_data[0] = port >> 8;
	sa.sa_data[1] = port & 0xff;
	sa.sa_data[2] = (ip.ipv4 >> 24) & 0xff;
	sa.sa_data[3] = (ip.ipv4 >> 16) & 0xff;
	sa.sa_data[4] = (ip.ipv4 >> 8) & 0xff;
	sa.sa_data[5] = ip.ipv4 & 0xff;

	int error = socket_connect(this->sock, &sa, sizeof(sa));

	if( error != 0 )
	{
		return error;
	}

	do
	{
		error = mbedtls_ssl_handshake( &ssl );
	}
	while( error == MBEDTLS_ERR_SSL_WANT_READ || error == MBEDTLS_ERR_SSL_WANT_WRITE );

	if( error != 0 )
	{
		return error;
	}

	return mbedtls_ssl_get_verify_result( &ssl );
}

int TLS::write( const unsigned char *buf, size_t len )
{
	const size_t original_len = len;
	int ret = mbedtls_ssl_write( &ssl, buf, len );
	while( (ret >= 0 && ret < len) || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
	{
		buf += ret;
		len -= ret;
		while( ret == 0 || ret == MBEDTLS_ERR_SSL_WANT_WRITE )
		{
			ret = mbedtls_ssl_write( &ssl, buf, len );
		}
	}

	if( ret < 0 && ret != MBEDTLS_ERR_SSL_WANT_READ )
	{
		int ret2 = mbedtls_ssl_session_reset( &ssl );
		if( ret2 < 0 )
		{
			// conundrum, we have 2 errors now, could return either
			return ret2;
		}
		return ret;
	}
	else
	{
		return original_len;
	}
}

int TLS::read( unsigned char *buf, size_t len )
{
	memset( buf, 0, len );
	const size_t original_len = len;
	int ret = mbedtls_ssl_read( &ssl, buf, len );
	while( (ret > 0 && ret < len) || ret == MBEDTLS_ERR_SSL_WANT_READ )
	{
		buf += ret;
		len -= ret;
		while( ret == MBEDTLS_ERR_SSL_WANT_READ )
		{
			ret = mbedtls_ssl_read( &ssl, buf, len );
		}
	}

	if( ret < 0 && ret != MBEDTLS_ERR_SSL_WANT_WRITE )
	{
		int ret2 = mbedtls_ssl_session_reset( &ssl );
		if( ret2 < 0 )
		{
			// conundrum, we have 2 errors now, could return either
			return ret2;
		}
		return ret;
	}
	else
	{
		return original_len;
	}
}

int TLS::close()
{
	socket_close(this->sock);
	return mbedtls_ssl_close_notify( &ssl );
}

int TLS::send( void *ctx, const unsigned char *buf, size_t len )
{
	TLS *self = (TLS *)ctx;
	return socket_send(self->sock, buf, len);
}

int TLS::recv( void *ctx, unsigned char *buf, size_t len, uint32_t timeout )
{
	TLS *self = (TLS *)ctx;
	return socket_receive(self->sock, buf, len, timeout);
}

int TLS::collect_entropy( void *data, unsigned char *output, size_t len, size_t *olen )
{
	unsigned char *p = output;
	unsigned char const * const end = output + len - 4;
	uint32_t next;

	while( p <= end )
	{
		next = HAL_RNG_GetRandomNumber();
		*p++ = next & 0xff;
		next >>= 8;
		*p++ = next & 0xff;
		next >>= 8;
		*p++ = next & 0xff;
		next >>= 8;
		*p++ = next & 0xff;
	}

	*olen = p - output;

	return 0;
}
