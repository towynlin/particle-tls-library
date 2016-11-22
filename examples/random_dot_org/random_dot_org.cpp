#include "tls.h"

static volatile char currentRandomHexString[1024];

TLS tls;

void logFailure(const char *whichCall, int16_t error) {
	if (error != 0) {
		String msg = String("TLS ") + String(whichCall);
		msg += String(" failed with error ") + String(error);
		Serial.println(msg);
		Particle.publish("tls-fail", msg, 60, PRIVATE);
	}
}

int updateRandomHexString(String _) {
	int16_t error = tls.connect("www.random.org", "443");
	logFailure("connect", error);

	tls.write("GET /cgi-bin/randbyte HTTP/1.1\r\n"
	          "Host: www.random.org\r\n\r\n");
	int charsReadOrError = tls.read(currentRandomHexString, 1024);
	Particle.publish("tls-chars-read-or-error", charsReadOrError, 300000, PRIVATE);
	tls.close();
	return charsReadOrError;
}

void setup() {
	Serial.begin();
	Particle.variable(currentRandomHexString);
	Particle.function("update-rand", updateRandomHexString);

	int16_t error = tls.init();
	logFailure("init", error);
}

void loop() {
	// update random hex string every 5 minutes
	static unsigned long t = millis();
	if (millis() - t > 300000) {
		t = millis();
		updateRandomHexString(String());
	}
}
