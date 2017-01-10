#include "application.h"
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

const size_t HEX_STR_LEN = 513;
char currentRandomHexString[HEX_STR_LEN];
const size_t BUF_LEN = 1024;
unsigned char readBuf[BUF_LEN];
const char HEX_CHARS[] = {
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

TLS tls;

void logFailure(const char *whichCall, int error) {
	if (error != 0) {
		String msg = String("TLS ") + String(whichCall);
		msg += String(" failed with error ") + String(error);
		Serial.println(msg);
		Particle.publish("tls-fail", msg, 60, PRIVATE);
	}
}

void populateHexStr(unsigned char *inBuf, size_t inLen, char *outBuf, size_t outLen) {
	unsigned char *p = inBuf;

	// first, find the start of the HTTP body
	bool foundBody = false;
	// require at least one byte of HTTP body
	const unsigned char *inEndCheck = inBuf + inLen - 5;
	while (p < inEndCheck && !foundBody) {
		if (*p++ != '\r') {
			continue;
		}
		if (*p == '\n' && *(p+1) == '\r' && *(p+2) == '\n') {
			foundBody = true;
			p += 3;
		}
	}
	if (!foundBody) {
		return;
	}

	// second, hex encode the body
	inEndCheck = inBuf + inLen - 1;
	// allow for a terminating null byte
	const char *outEndCheck = outBuf + outLen - 3;
	char *q = outBuf;
	while (p <= inEndCheck && q <= outEndCheck) {
		*q++ = HEX_CHARS[*p >> 4];
		*q++ = HEX_CHARS[*p & 0xf];
		p++;
	}
	*q = 0;
}

int updateRandomHexString(String _) {
	int error = tls.connect("www.random.org", 443);
	logFailure("connect", error);

	const char *request_ascii = 
		"GET /cgi-bin/randbyte HTTP/1.1\r\n"
		"Host: www.random.org\r\n\r\n";
	const unsigned char *request_binary =
		reinterpret_cast<const unsigned char *>(request_ascii);
	tls.write(request_binary, strlen(request_ascii));
	int charsReadOrError = tls.read(readBuf, BUF_LEN);
	Particle.publish("tls-chars-read-or-error", String(charsReadOrError), 300000, PRIVATE);
	populateHexStr(readBuf, BUF_LEN, currentRandomHexString, HEX_STR_LEN);
	tls.close();
	return charsReadOrError;
}

void setup() {
	Serial.begin();
	currentRandomHexString[HEX_STR_LEN-1] = 0;
	Particle.variable("rand", currentRandomHexString);
	Particle.function("update-rand", updateRandomHexString);

	int error = tls.init(root_pem, sizeof(root_pem));
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
