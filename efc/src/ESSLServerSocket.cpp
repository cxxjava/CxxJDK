/*
 * ESSLServerSocket.cpp
 *
 *  Created on: 2017-1-11
 *      Author: cxxjava@163.com
 */

#include "ESSLServerSocket.hh"
#include "ESSLCommon.hh"
#include "ENetWrapper.hh"
#include "EToDoException.hh"

#ifdef HAVE_OPENSSL

namespace efc {

#ifndef OPENSSL_NO_NEXTPROTONEG
static int next_proto_cb(SSL *ssl, const unsigned char **out,
                         unsigned int *outlen, void *arg) {
	(void) ssl;

	EString* protocols = static_cast<EString*>(arg);

	*out = (unsigned char *) protocols->c_str();
	*outlen = protocols->length();

	return SSL_TLSEXT_ERR_OK;
}
#endif /* !OPENSSL_NO_NEXTPROTONEG */

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
static int alpn_select_proto_cb(SSL *ssl, const unsigned char **out,
                                unsigned char *outlen, const unsigned char *in,
                                unsigned int inlen, void *arg) {
	(void) ssl;

	EString* protocols = static_cast<EString*>(arg);

	unsigned char *srv = (unsigned char *) protocols->c_str();
	unsigned int srvlen = protocols->length();

	if (SSL_select_next_proto((unsigned char **) out, outlen, srv, srvlen, in,
			inlen) != OPENSSL_NPN_NEGOTIATED) {
		return SSL_TLSEXT_ERR_NOACK;
	}

	return SSL_TLSEXT_ERR_OK;
}
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

/* The passwd_cb() function must write the password into the provided buffer buf which is of length 'size' */
static int passwd_cb(char* buf, int size, int rwflag, void* password)
{
   int written = strlen((const char*)password);
   if (written > size) { // error
	   buf[0] = 0;
	   return -1;
   }
   memset(buf, 0, size);
   memcpy(buf, (const char*)password, written);
   return written;
}

static RSA * ssl_rsa512_key_callback(SSL *ssl, int is_export, int key_length) {
	static RSA *key;

	if (key_length == 512) {
		if (key == NULL) {
			key = RSA_generate_key(512, RSA_F4, NULL, NULL);
		}
	}

	return key;
}

//@see: https://wiki.openssl.org/index.php/OpenSSL_1.1.0_Changes
#if OPENSSL_VERSION_NUMBER < 0x10100000L
static int DH_set0_pqg(DH *dh, BIGNUM *p, BIGNUM *q, BIGNUM *g) {
	/* If the fields p and g in d are NULL, the corresponding input
	 * parameters MUST be non-NULL.  q may remain NULL.
	 */
	if ((dh->p == NULL && p == NULL) || (dh->g == NULL && g == NULL))
		return 0;

	if (p != NULL) {
		BN_free(dh->p);
		dh->p = p;
	}
	if (q != NULL) {
		BN_free(dh->q);
		dh->q = q;
	}
	if (g != NULL) {
		BN_free(dh->g);
		dh->g = g;
	}

	if (q != NULL) {
		dh->length = BN_num_bits(q);
	}

	return 1;
}
#endif //

boolean ESSLServerSocket::useDHFile(const char* dh_file, CipherSuiteModel ciphersuite_model) {
	/**
	 * When an ephemeral Diffie-Hellman cipher is used, the server and the client negotiate a pre-master key using the Diffie-Hellman algorithm.
	 * This algorithm requires that the server sends the client a prime number and a generator. Neither are confidential, and are sent in clear
	 * text. However, they must be signed, such that a MITM cannot hijack the handshake.
	 * As an example, TLS_DHE_RSA_WITH_AES_128_CBC_SHA256 works as follow:
	 *
	 * 1. Server sends Client a [SERVER KEY EXCHANGE] message during the SSL Handshake. The message contains:
	 *    Prime number p
	 *    Generator g
	 *    Server's Diffie-Hellman public value A = g^X mod p, where X is a private integer chosen by the server at random, and never shared with the client
	 *    signature S of the above (plus two random values) computed using the Server's private RSA key
	 *
	 * 2. Client verifies the signature S
	 * 3. Client sends server a [CLIENT KEY EXCHANGE] message. The message contains:
	 *    Client's Diffie-Hellman public value B = g^Y mod p, where Y is a private integer chosen at random and never shared
	 *
	 * 4. The Server and the Client can now calculate the pre-master secret using each other's public values:
	 *    server calculates PMS = B^X mod p
	 *    client calculates PMS = A^Y mod p
	 *
	 * 5. Client sends a [CHANGE CIPHER SPEC] message to the server, and both parties continue the handshake using ENCRYPTED HANDSHAKE MESSAGES
	 *
	 * The size of the prime number p constrains the size of the pre-master key PMS, because of the modulo operation. A smaller prime almost
	 * means weaker values of A and B, which could leak the secret values X and Y. Thus, the prime p should not be smaller than the size of
	 * the RSA private key
	 */

	DH* dh = 0;

	if (dh_file && *dh_file) {
		FILE* paramfile = (FILE*) fopen(dh_file, "r");

		if (paramfile == 0)
			return false;

		dh = (DH*) PEM_read_DHparams(paramfile, 0, 0, 0);

		(void) fclose(paramfile);
	} else {
		/**
		 * The concept of forward secrecy is simple: client and server negotiate a key that never hits the wire,
		 * and is destroyed at the end of the session. The RSA private from the server is used to sign a Diffie-Hellman
		 * key exchange between the client and the server. The pre-master key obtained from the Diffie-Hellman handshake
		 * is then used for encryption. Since the pre-master key is specific to a connection between a client and a server,
		 * and used only for a limited amount of time, it is called Ephemeral. With Forward Secrecy, if an attacker gets a
		 * hold of the server's private key, it will not be able to decrypt past communications. The private key is only
		 * used to sign the DH handshake, which does not reveal the pre-master key. Diffie-Hellman ensures that the pre-master
		 * keys never leave the client and the server, and cannot be intercepted by a MITM
		 */
		static unsigned char dhxxx2_g[] = {0x02};
		static unsigned char dh1024_p[] = {
			0xA2,0x95,0x7E,0x7C,0xA9,0xD5,0x55,0x1D,0x7C,0x77,0x11,0xAC,
			0xFD,0x48,0x8C,0x3B,0x94,0x1B,0xC5,0xC0,0x99,0x93,0xB5,0xDC,
			0xDC,0x06,0x76,0x9E,0xED,0x1E,0x3D,0xBB,0x9A,0x29,0xD6,0x8B,
			0x1F,0xF6,0xDA,0xC9,0xDF,0xD5,0x02,0x4F,0x09,0xDE,0xEC,0x2C,
			0x59,0x1E,0x82,0x32,0x80,0x9B,0xED,0x51,0x68,0xD2,0xFB,0x1E,
			0x25,0xDB,0xDF,0x9C,0x11,0x70,0xDF,0xCA,0x19,0x03,0x3D,0x3D,
			0xC1,0xAC,0x28,0x88,0x4F,0x13,0xAF,0x16,0x60,0x6B,0x5B,0x2F,
			0x56,0xC7,0x5B,0x5D,0xDE,0x8F,0x50,0x08,0xEC,0xB1,0xB9,0x29,
			0xAA,0x54,0xF4,0x05,0xC9,0xDF,0x95,0x9D,0x79,0xC6,0xEA,0x3F,
			0xC9,0x70,0x42,0xDA,0x90,0xC7,0xCC,0x12,0xB9,0x87,0x86,0x39,
			0x1E,0x1A,0xCE,0xF7,0x3F,0x15,0xB5,0x2B};
		static unsigned char dh2048_p[] = {
			0xF2,0x4A,0xFC,0x7E,0x73,0x48,0x21,0x03,0xD1,0x1D,0xA8,0x16,
			0x87,0xD0,0xD2,0xDC,0x42,0xA8,0xD2,0x73,0xE3,0xA9,0x21,0x31,
			0x70,0x5D,0x69,0xC7,0x8F,0x95,0x0C,0x9F,0xB8,0x0E,0x37,0xAE,
			0xD1,0x6F,0x36,0x1C,0x26,0x63,0x2A,0x36,0xBA,0x0D,0x2A,0xF5,
			0x1A,0x0F,0xE8,0xC0,0xEA,0xD1,0xB5,0x52,0x47,0x1F,0x9A,0x0C,
			0x0F,0xED,0x71,0x51,0xED,0xE6,0x62,0xD5,0xF8,0x81,0x93,0x55,
			0xC1,0x0F,0xB4,0x72,0x64,0xB3,0x73,0xAA,0x90,0x9A,0x81,0xCE,
			0x03,0xFD,0x6D,0xB1,0x27,0x7D,0xE9,0x90,0x5E,0xE2,0x10,0x74,
			0x4F,0x94,0xC3,0x05,0x21,0x73,0xA9,0x12,0x06,0x9B,0x0E,0x20,
			0xD1,0x5F,0xF7,0xC9,0x4C,0x9D,0x4F,0xFA,0xCA,0x4D,0xFD,0xFF,
			0x6A,0x62,0x9F,0xF0,0x0F,0x3B,0xA9,0x1D,0xF2,0x69,0x29,0x00,
			0xBD,0xE9,0xB0,0x9D,0x88,0xC7,0x4A,0xAE,0xB0,0x53,0xAC,0xA2,
			0x27,0x40,0x88,0x58,0x8F,0x26,0xB2,0xC2,0x34,0x7D,0xA2,0xCF,
			0x92,0x60,0x9B,0x35,0xF6,0xF3,0x3B,0xC3,0xAA,0xD8,0x58,0x9C,
			0xCF,0x5D,0x9F,0xDB,0x14,0x93,0xFA,0xA3,0xFA,0x44,0xB1,0xB2,
			0x4B,0x0F,0x08,0x70,0x44,0x71,0x3A,0x73,0x45,0x8E,0x6D,0x9C,
			0x56,0xBC,0x9A,0xB5,0xB1,0x3D,0x8B,0x1F,0x1E,0x2B,0x0E,0x93,
			0xC2,0x9B,0x84,0xE2,0xE8,0xFC,0x29,0x85,0x83,0x8D,0x2E,0x5C,
			0xDD,0x9A,0xBB,0xFD,0xF0,0x87,0xBF,0xAF,0xC4,0xB6,0x1D,0xE7,
			0xF9,0x46,0x50,0x7F,0xC3,0xAC,0xFD,0xC9,0x8C,0x9D,0x66,0x6B,
			0x4C,0x6A,0xC9,0x3F,0x0C,0x0A,0x74,0x94,0x41,0x85,0x26,0x8F,
			0x9F,0xF0,0x7C,0x0B};

		dh = (DH*)DH_new();

		BIGNUM *p, *g;
		switch (ciphersuite_model)
		{
			case Modern: p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), 0); break;
			case Old: p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), 0); break;
			default: p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), 0); break; // Intermediate
		}
		ES_ASSERT(p);
		g = BN_bin2bn(dhxxx2_g, sizeof(dhxxx2_g), 0);
		ES_ASSERT(g);
		DH_set0_pqg(dh, p, NULL, g); //!
	}

	if (dh == 0)
		return false;

	(void) SSL_CTX_set_tmp_dh(ctx, dh);

	DH_free(dh);

	return true;
}

void ESSLServerSocket::init() {
	if (!ctx) {
		ctx = SSL_CTX_new(SSLv23_method());

		SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

		SSL_CTX_set_options(ctx, SSL_OP_ALL);

		SSL_CTX_set_read_ahead(ctx, 1);

		SSL_CTX_set_timeout(ctx, 300); //300s

		// @see ulib sslsocket.cpp#L179
		SSL_CTX_set_quiet_shutdown(ctx, 1);

#ifdef SSL_MODE_RELEASE_BUFFERS
		SSL_CTX_set_mode(ctx, SSL_MODE_RELEASE_BUFFERS);
#endif //@see: nginx-1.15.1/src/event/ngx_event_openssl.c#337
	}
}

ESSLServerSocket::~ESSLServerSocket() {
	if (ctx) SSL_CTX_free(ctx);
	if (store) X509_STORE_free(store);
}

ESSLServerSocket::ESSLServerSocket() :
		EServerSocket(), ctx(null), store(null) {
	init();
}

ESSLServerSocket::ESSLServerSocket(int port, int backlog, EInetAddress *bindAddr) :
		EServerSocket(port, backlog, bindAddr), ctx(null) {
	init();
}

//@see: ULib/src/ulib/ssl/net/sslsocket.cpp::setContext()
boolean ESSLServerSocket::setSSLParameters(const char* cert_file,
		const char* private_key_file, const char* passwd, const char* CAfile,
		const char* DHfile, CipherSuiteModel ciphersuite_model) {
	int result = 0;

	//@see: https://blog.csdn.net/mrpre/article/details/78025940

	// These are the bit DH parameters from "Assigned Number for SKIP Protocols"
	// See there for how they were generated: http://www.skip-vpn.org/spec/numbers.html
	if (useDHFile(DHfile, ciphersuite_model) == false) {
		return false;
	}

	// Set ecdh, if not set then Chrome show ERR_SPDY_INADEQUATE_TRANSPORT_SECURITY.
	EC_KEY *ecdh = EC_KEY_new_by_curve_name(NID_X9_62_prime256v1);
	if (!ecdh) {
		return false;
	}
	SSL_CTX_set_tmp_ecdh(ctx, ecdh);
	EC_KEY_free(ecdh);

#if (OPENSSL_VERSION_NUMBER < 0x10100001L && !defined LIBRESSL_VERSION_NUMBER)
    /* a temporary 512-bit RSA key is required for export versions of MSIE */
    SSL_CTX_set_tmp_rsa_callback(ctx, ssl_rsa512_key_callback);
#endif

	// Load CERT PEM file

	if (cert_file && *cert_file) {
		result = SSL_CTX_use_certificate_chain_file(ctx, cert_file);

		if (result == 0)
			return false;
	}

	// Load private key PEM file and give passwd callback if any

	if (private_key_file && *private_key_file) {
		if (passwd && *passwd) {
			SSL_CTX_set_default_passwd_cb(ctx, passwd_cb);
			SSL_CTX_set_default_passwd_cb_userdata(ctx, (void*) passwd);
		}

		for (int i = 0; i < 3; ++i) {
			result = SSL_CTX_use_PrivateKey_file(ctx, private_key_file,
					SSL_FILETYPE_PEM);

			if (result)
				break;

			unsigned long error = ERR_peek_error();

			if (ERR_GET_REASON(error) == EVP_R_BAD_DECRYPT) {
				if (i < 2) { // Give the user two tries
					(void) ERR_get_error(); // remove from stack
					continue;
				}
			}

			return false;
		}

		// Check private key

		result = SSL_CTX_check_private_key(ctx);

		if (result == 0)
			return false;
	}

	if (CAfile && *CAfile) {
		// Verify client cert
		SSL_CTX_set_verify(ctx, (SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT), NULL);
		SSL_CTX_set_verify_depth(ctx, 1);
		SSL_CTX_load_verify_locations(ctx, CAfile, NULL);

		//
		if (store) X509_STORE_free(store);
		store = X509_STORE_new();

		if (X509_STORE_set_default_paths(store) == 0 || // add a lookup file/method to an X509 store
				X509_STORE_load_locations(store, CAfile, NULL) == 0) {
			return false;
		}
		X509_STORE_set_flags(store, X509_V_FLAG_CRL_CHECK | X509_V_FLAG_CRL_CHECK_ALL);

		// Sets the list of CA sent to the client when requesting a client certificate for ctx
		STACK_OF(X509_NAME)* list =
				(STACK_OF(X509_NAME)*) SSL_load_client_CA_file(CAfile);

		/*
		 * before 0.9.7h and 0.9.8 SSL_load_client_CA_file()
		 * always leaved an error in the error queue
		 */
		ERR_clear_error();

		SSL_CTX_set_client_CA_list(ctx, list);
	}

	/**
	 * see: https://wiki.mozilla.org/Security/Server_Side_TLS
	 *
	 * Three configurations are recommended. Pick the right configuration depending on your audience. If you do not need
	 * backward compatibility, and are building a service for modern clients only (post FF27), then use the Modern configuration.
	 * Otherwise, prefer the Intermediate configuration. Use the Old backward compatible configuration only if your service will
	 * be accessed by very old clients, such as Windows XP IE6, or ancient libraries & bots.
	 *
	 * Modern       Firefox 27, Chrome 22, IE 11, Opera 14, Safari 7, Android 4.4, Java 8
	 * Intermediate Firefox 1, Chrome 1, IE 7, Opera 5, Safari 1, Windows XP IE8, Android 2.3, Java 7
	 * Old          Windows XP IE6, Java 6
	 */

	const char* ciphersuite;
	int options = SSL_CTX_get_options(ctx);

	switch (ciphersuite_model) {
	case Modern: {
		options |= SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3 | SSL_OP_NO_TLSv1;

		ciphersuite =
				"ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:"
						"ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:"
						"ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:"
						"ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:"
						"DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:"
						"DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
	}
		break;

	case Old: {
		options |= SSL_OP_NO_SSLv2;

		ciphersuite =
				"ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:"
						"ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:"
						"ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:"
						"ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:"
						"DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:"
						"DHE-RSA-AES256-SHA:ECDHE-RSA-DES-CBC3-SHA:ECDHE-ECDSA-DES-CBC3-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:"
						"AES128-SHA:AES256-SHA:AES:DES-CBC3-SHA:HIGH:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:"
						"!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
	}
		break;

	default: // Intermediate
	{
		options |= SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3;

		ciphersuite =
				"ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:"
						"DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:"
						"ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:"
						"ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:"
						"DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA:AES256-SHA:AES:CAMELLIA:"
						"DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
	}
		break;
	}

#ifdef SSL_OP_NO_COMPRESSION
	(void) SSL_CTX_set_options(ctx, options |
			SSL_OP_NO_COMPRESSION |
			SSL_OP_CIPHER_SERVER_PREFERENCE); // SSLHonorCipherOrder On - determine SSL cipher in server-preferred order, not client-order
#else
	(void) SSL_CTX_set_options(ctx, options |
			SSL_OP_CIPHER_SERVER_PREFERENCE); // SSLHonorCipherOrder On - determine SSL cipher in server-preferred order, not client-order
#endif

	(void) SSL_CTX_set_cipher_list(ctx, ciphersuite);

    /* session cache set */
    {
    	SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
    	int session_id_context = 0;
    	SSL_CTX_set_session_id_context(ctx, (const unsigned char *)&session_id_context, sizeof(session_id_context));
    	SSL_CTX_sess_set_cache_size(ctx, 1);
    }

	return true;
}

void ESSLServerSocket::setNegotiatedProtocols(const char* protocol, ...) {
	// clear old.
	protocols.clear();

	if (protocol && *protocol) {
		// set new cb

		va_list args;
		va_start(args, protocol);

		char* arg = (char*)protocol;
		do {
			protocols.append((char)strlen(arg));  // length
			protocols.append(arg);          // text
		} while ((arg = va_arg(args, char*)));

		va_end(args);
	}

	setSSLNextProtos();
}

void ESSLServerSocket::setSSLNextProtos() {
	if (!protocols.isEmpty()) {
		// set new cb

#ifndef OPENSSL_NO_NEXTPROTONEG
		SSL_CTX_set_next_protos_advertised_cb(ctx, next_proto_cb, &protocols);
#endif /* !OPENSSL_NO_NEXTPROTONEG */

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
		SSL_CTX_set_alpn_select_cb(ctx, alpn_select_proto_cb, &protocols);
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */

	} else {
		// clear cb

#ifndef OPENSSL_NO_NEXTPROTONEG
		SSL_CTX_set_next_protos_advertised_cb(ctx, NULL, NULL);
#endif /* !OPENSSL_NO_NEXTPROTONEG */

#if OPENSSL_VERSION_NUMBER >= 0x10002000L
		SSL_CTX_set_alpn_select_cb(ctx, NULL, NULL);
#endif /* OPENSSL_VERSION_NUMBER >= 0x10002000L */
	}
}

ESSLSocket* ESSLServerSocket::accept() {
	if (isClosed())
		throw ESocketException(__FILE__, __LINE__, "Socket is closed");
	if (!isBound())
		throw ESocketException(__FILE__, __LINE__, "Socket is not bound yet");

	struct ip_addr raddr;
	int rport;
	int newsock = eso_net_accept(socket, &raddr, &rport);
	if (newsock < 0) {
		if (newsock == -2) {
			throw ESocketTimeoutException(__FILE__, __LINE__, "Accept timed out");
		} else {
			throw ESocketException(__FILE__, __LINE__, "Accept failed");
		}
	}

	// ssl init

	ERR_clear_error();

	SSL *ssl;
	{
		ssl = SSL_new(ctx);
		if (!ssl) {
			eso_net_close(newsock);
			throw EIOException(__FILE__, __LINE__, ESSLCommon::getErrors().c_str());
		}

		if (!SSL_set_fd(ssl, newsock)) {
			SSL_free(ssl);
			eso_net_close(newsock);
			throw EIOException(__FILE__, __LINE__, ESSLCommon::getErrors().c_str());
		}

		int timeout = getSoTimeout();
		if (timeout > 0) {
			ENetWrapper::configureBlocking(newsock, false);
			{
				int status = -1;
				int flags = 0;

				do {
					status = SSL_accept(ssl);
					switch (SSL_get_error(ssl, status)) {
					case SSL_ERROR_NONE:
						status = 0; // To tell caller about success
						break; // Done
					case SSL_ERROR_WANT_WRITE:
						flags |= NET_WAIT_WRITE;
						status = 1; // Wait for more activity
						break;
					case SSL_ERROR_WANT_READ:
						flags |= NET_WAIT_READ;
						status = 1; // Wait for more activity
						break;
					case SSL_ERROR_ZERO_RETURN:
					case SSL_ERROR_SYSCALL:
						// The peer has notified us that it is shutting down via
						// the SSL "close_notify" message so we need to
						// shutdown, too.
						status = -1;
						break;
					default:
						status = -1;
						break;
					}

					if (status == 1) {
						// Must have at least one handle to wait for at this point.
						status =  eso_net_wait(newsock, flags, timeout);

						// 0 is timeout, so we're done.
						// -1 is error, so we're done.
						// Could be both handles set (same handle in both masks) so
						// set to 1.
						if (status >= 1) {
							status = 1;
						} else { // Timeout or failure
							status = -1;
						}
					}

				} while (status == 1 && !SSL_is_init_finished(ssl));

				if (status < 0) {
					SSL_free(ssl);
					eso_net_close(newsock);
					return null; //!
				}
			}
			ENetWrapper::configureBlocking(newsock, true);
		} else {
			RETRY:

			ERR_clear_error();

			int n, e;
			if ((n = SSL_accept(ssl)) != 1) {
				e = SSL_get_error(ssl, n);
				switch (e) {
				case SSL_ERROR_WANT_READ:
				case SSL_ERROR_WANT_WRITE:
					goto RETRY;
				default:
					SSL_free(ssl);
					eso_net_close(newsock);
					return null; //!
				}
			}
		}
	}

	//printf("Server with %s encryption\n", SSL_get_cipher(ssl));

	// if SSL accept success then the ssl object freed in ~ESSLSocket()!

	ESSLSocket *newSocket = null;
	try {
		newSocket = new ESSLSocket(ssl, newsock, true, true, raddr.ip.s_addr, rport);
	} catch (EThrowable& t) {
		eso_net_close(newsock);
		delete newSocket;
		throw ESocketException(__FILE__, __LINE__, "Implement accept socket failed");
	}
	return newSocket;
}

void ESSLServerSocket::close() {
	EServerSocket::close();
}

EA<EString*> ESSLServerSocket::getSupportedCipherSuites() {
	//TODO...
	throw EToDoException(__FILE__, __LINE__);
}

EA<EString*> ESSLServerSocket::getEnabledCipherSuites() {
	//TODO...
	throw EToDoException(__FILE__, __LINE__);
}

} /* namespace efc */
#endif //!HAVE_OPENSSL
