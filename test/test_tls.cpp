#include "catch.hpp"
#include "tls.h"

SCENARIO( "Successful connection" ) {
	GIVEN( "I have initialized TLS" ) {
		TLS tls;
		int16_t error = tls.init();

		REQUIRE( error == 0 );

		WHEN( "I call connect with a valid host and port" ) {
			error = tls.connect("www.random.org", "443");

			THEN( "the return value is zero" ) {
				REQUIRE( error == 0 );
			}
		}

		WHEN( "I call connect with a bad host" ) {
			error = tls.connect("gazorpa.zorp", "443");

			THEN( "the return value is -82" ) {
				REQUIRE( error == -82 );
			}
		}
	}
}
