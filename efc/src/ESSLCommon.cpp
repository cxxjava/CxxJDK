/*
 * ESSLCommon.cpp
 *
 *  Created on: 2017-1-14
 *      Author: cxxjava@163.com
 */

#include "ESSLCommon.hh"

#ifdef HAVE_OPENSSL

namespace efc {

//=============================================================================

//@see: https://github.com/openssl/openssl/issues/1260
/**
 * Removal of CRYPTO_LOCK breaks legacy locking callbacks #1260
 * CRYPTO_set_locking_callback is useless in 1.1.0, it's one of this interfaces we consiously
 * removed so as not to fool users to keep using those.
 */

#if OPENSSL_VERSION_NUMBER < 0x10100000L
static es_thread_mutex_t **ssl_global_locks;
static void ssl_locking_cb(int mode, int type, const char *file, int line) {
	if (mode & CRYPTO_LOCK) {
		eso_thread_mutex_lock(ssl_global_locks[type]);
	} else {
		eso_thread_mutex_unlock(ssl_global_locks[type]);
	}
}
#endif //

DEFINE_STATIC_INITZZ_BEGIN(ESSLCommon)
    OPENSSL_config(NULL);

    SSL_library_init();
    SSL_load_error_strings();

    OpenSSL_add_all_algorithms();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	// CRYPTO_set_id_callback(ssl_thread_id);
    // OpenSSL manual says that if threadid_func is not specified using
	// CRYPTO_THREADID_set_callback(), then default implementation is
	// used. We use this default one.
    int lock_num_locks = CRYPTO_num_locks();
    ssl_global_locks = (es_thread_mutex_t**)malloc(lock_num_locks * sizeof(es_thread_mutex_t*));
    for (int i = 0; i < lock_num_locks; i++) {
    	ssl_global_locks[i] = eso_thread_mutex_create(ES_THREAD_MUTEX_DEFAULT);
	}
	CRYPTO_set_locking_callback(ssl_locking_cb);
#endif //

#if OPENSSL_VERSION_NUMBER >= 0x0090800fL
#ifndef SSL_OP_NO_COMPRESSION
    {
    /*
     * Disable gzip compression in OpenSSL prior to 1.0.0 version,
     * this saves about 522K per connection.
     */
    int                  n;
    STACK_OF(SSL_COMP)  *ssl_comp_methods;

    ssl_comp_methods = SSL_COMP_get_compression_methods();
    n = sk_SSL_COMP_num(ssl_comp_methods);

    while (n--) {
        (void) sk_SSL_COMP_pop(ssl_comp_methods);
    }
    }
#endif
#endif
DEFINE_STATIC_INITZZ_END

//=============================================================================

EString ESSLCommon::getErrors() {
	EString errors;
	unsigned long errorCode;
	char message[256];

	while ((errorCode = ERR_get_error()) != 0) {
		if (!errors.isEmpty()) {
			errors += "; ";
		}
		const char* reason = ERR_reason_error_string(errorCode);
		if (reason == null) {
			eso_snprintf(message, sizeof(message) - 1, "SSL error # %lu",
					errorCode);
			reason = message;
		}
		errors += reason;
	}

	if (errors.isEmpty()) {
		errors << "SSL no error ?";
	}

	return errors;
}

/**
 * Examine OpenSSL's error stack, and return a string description of the
 * errors.
 *
 * This operation removes the errors from OpenSSL's error stack.
 */
EString ESSLCommon::getErrors(unsigned long errnoCopy) {
	EString errors;
	char message[256];

	do {
		if (!errors.isEmpty()) {
			errors += "; ";
		}
		const char* reason = ERR_reason_error_string(errnoCopy);
		if (reason == null) {
			eso_snprintf(message, sizeof(message) - 1, "SSL error # %lu",
					errnoCopy);
			reason = message;
		}
		errors += reason;
	} while ((errnoCopy = ERR_get_error()) != 0);

	return errors;
}

} /* namespace efc */
#endif //!HAVE_OPENSSL
