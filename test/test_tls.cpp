#include "catch.hpp"
#include "tls.h"

SCENARIO( "Writing and reading over TLS" ) {
	GIVEN( "I have initialized TLS" ) {
		TLS tls;
		int error = tls.init();

		REQUIRE( error == 0 );

		WHEN( "I call connect with a valid host and port" ) {
			error = tls.connect("www.random.org", "443");

			THEN( "the return value is zero" ) {
				REQUIRE( error == 0 );

				AND_WHEN( "I call write with a valid HTTP request" ) {
					error = tls.write("GET /cgi-bin/randbyte HTTP/1.1\r\n"
					                  "Host: www.random.org\r\n\r\n");

					THEN( "the socket has data to read" ) {
						REQUIRE( error == MBEDTLS_ERR_SSL_WANT_READ );
					}
				}
			}
		}

		WHEN( "I call connect with a bad host" ) {
			error = tls.connect("gazorpa.zorp", "443");

			THEN( "the return value is MBEDTLS_ERR_NET_UNKNOWN_HOST" ) {
				REQUIRE( error == MBEDTLS_ERR_NET_UNKNOWN_HOST );
			}
		}
	}
}
