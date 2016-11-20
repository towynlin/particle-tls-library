#include "catch.hpp"
#include "tls.h"

SCENARIO( "Successful initialization" ) {
	GIVEN( "a TLS instance" ) {
		TLS tls;
		WHEN( "I call init" ) {
			int16_t error = tls.init();
			THEN( "the return value is zero" ) {
				REQUIRE( error == 0 );
			}
		}
	}
}
