#include "tls.h"

static volatile char currentRandomHexString[1024];

TLS tls;
int16_t tlsInitError;

int updateRandomHexString(String _) {
	tls.connect("www.random.org", 443);
	tls.write("GET /cgi-bin/randbyte HTTP/1.1\r\n"
	          "Host: www.random.org\r\n\r\n");
	int charsReadOrError = tls.read(currentRandomHexString, 1024);
	Particle.publish("tls-chars-read-or-error", charsReadOrError, 300000, PRIVATE);
	tls.close();
	return charsReadOrError;
}

void logInitFailure() {
	Serial.print("TLS init failed with error ");
	Serial.println(tlsInitError);
	Particle.publish("tls-init-fail", String(tlsInitError), 60, PRIVATE);
}

void setup() {
	Serial.begin();
	Particle.variable(currentRandomHexString);
	Particle.function("update-rand", updateRandomHexString);

	tlsInitError = tls.init();
	if (tlsInitError != 0) {
		logInitFailure();
	}
}

void loop() {
	// update random hex string every 5 minutes
	static unsigned long t = millis();
	if (millis() - t > 300000) {
		t = millis();
		updateRandomHexString(String());
	}
}
