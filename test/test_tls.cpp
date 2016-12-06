#include "catch.hpp"
#include "tls.h"

#define DIGICERT_HIGH_ASSURANCE_EV_ROOT_CA                             \
"-----BEGIN CERTIFICATE-----\r\n"                                      \
"MIIDxTCCAq2gAwIBAgIQAqxcJmoLQJuPC3nyrkYldzANBgkqhkiG9w0BAQUFADBs\r\n" \
"MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3\r\n" \
"d3cuZGlnaWNlcnQuY29tMSswKQYDVQQDEyJEaWdpQ2VydCBIaWdoIEFzc3VyYW5j\r\n" \
"ZSBFViBSb290IENBMB4XDTA2MTExMDAwMDAwMFoXDTMxMTExMDAwMDAwMFowbDEL\r\n" \
"MAkGA1UEBhMCVVMxFTATBgNVBAoTDERpZ2lDZXJ0IEluYzEZMBcGA1UECxMQd3d3\r\n" \
"LmRpZ2ljZXJ0LmNvbTErMCkGA1UEAxMiRGlnaUNlcnQgSGlnaCBBc3N1cmFuY2Ug\r\n" \
"RVYgUm9vdCBDQTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMbM5XPm\r\n" \
"+9S75S0tMqbf5YE/yc0lSbZxKsPVlDRnogocsF9ppkCxxLeyj9CYpKlBWTrT3JTW\r\n" \
"PNt0OKRKzE0lgvdKpVMSOO7zSW1xkX5jtqumX8OkhPhPYlG++MXs2ziS4wblCJEM\r\n" \
"xChBVfvLWokVfnHoNb9Ncgk9vjo4UFt3MRuNs8ckRZqnrG0AFFoEt7oT61EKmEFB\r\n" \
"Ik5lYYeBQVCmeVyJ3hlKV9Uu5l0cUyx+mM0aBhakaHPQNAQTXKFx01p8VdteZOE3\r\n" \
"hzBWBOURtCmAEvF5OYiiAhF8J2a3iLd48soKqDirCmTCv2ZdlYTBoSUeh10aUAsg\r\n" \
"EsxBu24LUTi4S8sCAwEAAaNjMGEwDgYDVR0PAQH/BAQDAgGGMA8GA1UdEwEB/wQF\r\n" \
"MAMBAf8wHQYDVR0OBBYEFLE+w2kD+L9HAdSYJhoIAu9jZCvDMB8GA1UdIwQYMBaA\r\n" \
"FLE+w2kD+L9HAdSYJhoIAu9jZCvDMA0GCSqGSIb3DQEBBQUAA4IBAQAcGgaX3Nec\r\n" \
"nzyIZgYIVyHbIUf4KmeqvxgydkAQV8GK83rZEWWONfqe/EW1ntlMMUu4kehDLI6z\r\n" \
"eM7b41N5cdblIZQB2lWHmiRk9opmzN6cN82oNLFpmyPInngiK3BD41VHMWEZ71jF\r\n" \
"hS9OMPagMRYjyOfiZRYzy78aG6A9+MpeizGLYAiJLQwGXFK3xPkKmNEVX58Svnw2\r\n" \
"Yzi9RKR/5CYrCsSXaQ3pjOLAEFe4yHYSkVXySGnYvCoCWw9E1CAx2/S6cCZdkGCe\r\n" \
"vEsXCS+0yx5DaMkHJ8HSXPfqIbloEpw8nL+e/IBcm2PN7EeqJSdnoDfzAIJ9VNep\r\n" \
"+OkuE6N36B9K\r\n"                                                     \
"-----END CERTIFICATE-----\r\n"
const char root_pem[] = DIGICERT_HIGH_ASSURANCE_EV_ROOT_CA;

SCENARIO( "Writing and reading over TLS" ) {
	GIVEN( "I have initialized TLS" ) {
		TLS tls;
		int error = tls.init(root_pem, sizeof(root_pem));

		CHECK( error == 0 );

		WHEN( "I call connect with a valid host and port" ) {
			error = tls.connect("www.random.org", "443");

			THEN( "the return value is zero" ) {
				if( error == MBEDTLS_ERR_NET_UNKNOWN_HOST ) {
					printf("  !!  These tests require internet access.\n");
					printf("  !!  It looks like you may not have it.\n");
					printf("  !!  The -82 error below is \"unknown host\".\n");
				}
				REQUIRE( error == 0 );

				AND_WHEN( "I call write with a valid HTTP request" ) {
					const char *buf =
						"GET /cgi-bin/randbyte HTTP/1.1\r\n"
						"Host: www.random.org\r\n\r\n";
					size_t len = strlen( buf );
					const unsigned char *ubuf =
						reinterpret_cast<const unsigned char *>( buf );
					error = tls.write( ubuf, len );

					THEN( "56 bytes are written" ) {
						REQUIRE( error == 56 );

						AND_WHEN( "I call read" ) {
							unsigned char recv_buf[20];
							len = sizeof( recv_buf );
							error = tls.read( recv_buf, len);

							THEN( "the response looks like HTTP" ) {
								// There was more to read, so we filled the buffer
								REQUIRE( error == len );

								// First line of response was HTTP
								int cmp = strncmp( (const char *)recv_buf,
								                   "HTTP/1.1 200 OK\r\n", 17 );
								REQUIRE( cmp == 0 );
							}
						}
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
