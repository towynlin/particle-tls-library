#include "tls.h"
#include <cstring>

int TLS::init( const char *root_pem )
{
	int error;
	mbedtls_net_init( &server_fd );
	mbedtls_ssl_init( &ssl );
	mbedtls_ssl_config_init( &conf );
	mbedtls_x509_crt_init( &cacert );
	mbedtls_ctr_drbg_init( &ctr_drbg );
	mbedtls_entropy_init( &entropy );
	const char *personalization = "particle-tls-library";
	error = mbedtls_ctr_drbg_seed( &ctr_drbg, mbedtls_entropy_func, &entropy,
	                               (const unsigned char *)personalization,
	                               strlen( personalization ));
	if( error < 0 )
	{
		return error;
	}

	return mbedtls_x509_crt_parse( &cacert,
	                               (const unsigned char *) root_pem,
	                               sizeof( root_pem ) );
}

int TLS::connect( const char *host, const char *port )
{
	uint32_t flags;
	int error = mbedtls_net_connect( &server_fd, host, port, MBEDTLS_NET_PROTO_TCP );

	if( error != 0 )
	{
		return error;
	}

	error = mbedtls_ssl_config_defaults( &conf,
	                                     MBEDTLS_SSL_IS_CLIENT,
	                                     MBEDTLS_SSL_TRANSPORT_STREAM,
	                                     MBEDTLS_SSL_PRESET_DEFAULT );

	if( error != 0 )
	{
		return error;
	}

	mbedtls_ssl_conf_authmode( &conf, MBEDTLS_SSL_VERIFY_OPTIONAL );
	mbedtls_ssl_conf_ca_chain( &conf, &cacert, NULL );
	mbedtls_ssl_conf_rng( &conf, mbedtls_ctr_drbg_random, &ctr_drbg );

	error = mbedtls_ssl_setup( &ssl, &conf );

	if( error != 0 )
	{
		return error;
	}

	error = mbedtls_ssl_set_hostname( &ssl, host );

	if( error != 0 )
	{
		return error;
	}

	mbedtls_ssl_set_bio( &ssl, &server_fd, mbedtls_net_send, mbedtls_net_recv, NULL );

	do
	{
		error = mbedtls_ssl_handshake( &ssl );
	}
	while( error == MBEDTLS_ERR_SSL_WANT_READ || error == MBEDTLS_ERR_SSL_WANT_WRITE );

	if( error != 0 )
	{
		return error;
	}

	flags = mbedtls_ssl_get_verify_result( &ssl );
	if( flags != 0 )
	{
		return flags;
	}

	return 0;
}

int TLS::write( const unsigned char *buf, size_t len )
{
	const size_t original_len = len;
	int ret = mbedtls_ssl_write( &ssl, buf, len );
	while( ret > 0 && ret < len )
	{
		buf += ret;
		len -= ret;
		while( ret == 0 || ret == MBEDTLS_ERR_SSL_WANT_READ ||
		       ret == MBEDTLS_ERR_SSL_WANT_WRITE )
		{
			ret = mbedtls_ssl_write( &ssl, buf, len );
		}
	}

	if( ret < 0 )
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
