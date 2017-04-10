#ifndef MBEDTLS_CFG_H
#define MBEDTLS_CFG_H

// mbedtls: avoid implicit declaration errors for memcpy / memset
#include <string.h>

// override platform-specific dependencies such as libc/alloc
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_MEMORY_BUFFER_ALLOC_C
#define MBEDTLS_PLATFORM_EXIT_ALT

// mbedtls features
#define MBEDTLS_SHA256_C

#endif

