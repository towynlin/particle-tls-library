# TLS Library for [Particle](https://www.particle.io/)

This library wraps [mbed TLS](https://tls.mbed.org) into a format that can be
easily used in the Particle ecosystem of IoT devices. It is appropriate for
contrained, embedded devices and should also work well on other systems.

This library's code is test-driven and comes with examples that may be run on a
[Photon (Wi-Fi)](https://store.particle.io/collections/photon),
[Electron (Cellular)](https://store.particle.io/collections/electron),
or other Particle dev kit.

You can get started using the [web IDE](https://build.particle.io/), click the
libraries icon in the sidebar and search for TLS.

You can also simply clone this repo and run `make`.

Run the tests with `make test`.

Build the examples with `make examples`.


## Usage

Declare the instance statically at the top of your program.

```
TLS tls;
```

In `setup()` call `tls.init()` and check the return value.

```
void setup() {
	int16_t error = tls.init();
	if (error < 0) {
		// Uh oh! Failure! The library won't work!
	} else if (error > 0) {
		// Partial success parsing root CA certificates.
		// Things may or may not work.
	} else {
		// error == 0
		// Woot! Success!
	}
}
```

Define some function that calls connect, write, read, and close.
You'll probably call this function from `loop()` or from a `Particle.function()`.

```
char currentRandomHexString[1024];

int updateRandomHexString() {
	tls.connect("www.random.org", "443");
	tls.write("GET /cgi-bin/randbyte HTTP/1.1\r\n"
	          "Host: www.random.org\r\n\r\n");
	int charsReadOrError = tls.read(currentRandomHexString, 1024);
	tls.close();
	return charsReadOrError;
}

```


## Troubleshooting

The return value from `init()`, when non-zero, is an mbed TLS
error code returned from either `mbedtls_ctr_drbg_seed()`
(where the only documented error value is `-0x0034` for
[MBEDTLS_ERR_CTR_DRBG_ENTROPY_SOURCE_FAILED](https://tls.mbed.org/api/ctr__drbg_8h.html#a15d1931ea5d133062cd93a3374a5bcf0))
or, more likely,
[`mbedtls_x509_crt_parse()`](https://tls.mbed.org/api/group__x509__module.html#ga033567483649030f7f859db4f4cb7e14).

There are 4 possible return values from `connect()`:
- 0: Success
- -82: Unknown host
- -68: Connection failed
- -66: Socket failed


## License

This repo is almost entirely released under the
[Apache License, Version 2.0.](https://www.apache.org/licenses/LICENSE-2.0).

The mbed TLS library (Apache 2.0 licensed) is copyright ARM Limited.

The Catch test framework (one file: test/catch.hpp) is released under the
Boost Software License, Version 1.0 and is copyright Two Blue Cubes Ltd.

All other files are copyright Zachary Crockett.
