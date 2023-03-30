#include "k2_common.h"

#include <sys/types.h>

#include "MersenneTwister.h"

#ifndef BYTE_ORDER
#undef LITTLE_ENDIAN
#define LITTLE_ENDIAN 1234
#undef BIG_ENDIAN
#define BIG_ENDIAN 4321
#if defined(_LITTLE_ENDIAN) || (defined(__BYTE_ORDER) && defined(__LITTLE_ENDIAN) && (__BYTE_ORDER == __LITTLE_ENDIAN))
#define BYTE_ORDER LITTLE_ENDIAN
#elif defined(_BIG_ENDIAN) || (defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && (__BYTE_ORDER == __BIG_ENDIAN))
#define BYTE_ORDER BIG_ENDIAN
#elif defined(WIN32)
#define BYTE_ORDER LITTLE_ENDIAN
#else
#error "Cannot determine byte order!"
#endif
#endif // !BYTE_ORDER

#ifndef WIN32
#include <inttypes.h>
#else
typedef unsigned __int8		uint8_t;
typedef unsigned __int32	uint32_t;
typedef __int32				int32_t;
#endif

void
MersenneTwister::init_genrand(uint32_t seed) {
	// See Knuth TAOCP Vol2. 3rd Ed. P.106 for multiplier.
	// In the previous versions, MSBs of the seed affect
	// only MSBs of the array mt[].
	// 2002/01/09 modified by Makoto Matsumoto
	mt[0]= seed;
	for (mti = 1; mti < MT_N; mti++)
		mt[mti] = (1812433253UL * (mt[mti-1] ^ (mt[mti-1] >> 30)) + mti); 
}

void
MersenneTwister::init_by_array(uint32_t init_key[], int key_length) {
	int i, j, k;
	init_genrand(19650218UL);
	i = 1; j = 0;
	k = (MT_N > key_length ? MT_N : key_length);
	for (; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1664525UL))
			+ init_key[j] + j; // non linear
		i++; j++;
		if (i >= MT_N) { mt[0] = mt[MT_N-1]; i = 1; }
		if (j >= key_length) j = 0;
	}
	for (k = MT_N-1; k; k--) {
		mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 30)) * 1566083941UL))
			- i; // non linear
		i++;
		if (i >= MT_N) { mt[0] = mt[MT_N-1]; i=1; }
	}

	mt[0] = 0x80000000UL; /* MSB is 1; assuring non-zero initial array */ 
}

void
MersenneTwister::genrand_buf(void *buf, uint32_t buflen) {
	uint8_t *b = (uint8_t *)buf;
	uint8_t *b_end = b + buflen;
	uint32_t align = ((uint32_t)(size_t)b) & 0x3;
	if (align != 0) {
		uint32_t r = genrand_uint32();
		for (unsigned int i = 0; (i < (4 - align)) && (b < b_end); i++) {
			*b++ = (r & 0xff);
			r >>= 8;
		}
	}
	while ((b_end - b) >= 4) {
#if BYTE_ORDER == LITTLE_ENDIAN
		// optimize for common case of intel platforms
		*(uint32_t *)b = genrand_uint32();
		b += 4;
#elif BYTE_ORDER == BIG_ENDIAN
		// the following could be made faster using assembler instructions
		uint32_t r = genrand_uint32();
		*b++ = r & 0xff;
		*b++ = (r >> 8) & 0xff;
		*b++ = (r >> 16) & 0xff;
		*b++ = (r >> 24) & 0xff;
#endif
	}
	align = ((uint32_t)(size_t)b) & 0x3;
	if (align != 0) {
		uint32_t r = genrand_uint32();
		for (unsigned int i = 0; (i < (4 - align)) && (b < b_end); i++) {
			*b++ = (r & 0xff);
			r >>= 8;
		}
	}
}

#include <stdio.h>

int
main() {
	int i;
	uint32_t init[4]={0x123, 0x234, 0x345, 0x456}, length=4;
	MersenneTwister *mt = K2_NEW(ctx_Singleton,  MersenneTwister)(init, length);
	printf("1000 outputs of genrand_int32()\n");
	for (i=0; i<1000; i++) {
		printf("%10lu ", mt->genrand_uint32());
		if (i%5==4) printf("\n");
	}
	printf("\n1000 outputs of genrand_real2()\n");
	for (i=0; i<1000; i++) {
		printf("%10.8f ", mt->genrand_real2());
		if (i%5==4) printf("\n");
	}
	K2_DELETE(mt);
	return 0;
}