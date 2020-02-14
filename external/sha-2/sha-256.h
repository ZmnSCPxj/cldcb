#ifndef SHA_2_SHA_256_H
#define SHA_2_SHA_256_H
#include<stdint.h>
#include<stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif /* !defined(__cplusplus) */

void calc_sha_256(uint8_t hash[32], const void *input, size_t len);

#ifdef __cplusplus
}
#endif /* !defined(__cplusplus) */

#endif /* !defined(SHA_2_SHA_256_H) */
