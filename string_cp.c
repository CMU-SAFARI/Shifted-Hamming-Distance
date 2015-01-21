/*
 Bit population count, $Revision$

 This program includes following functions:
 * lookup  --- lookup based
 * ssse3-1 --- SSSE3 using PSHUFB and PSADBW
 * ssse3-2 --- improved SSSE3 procedure - PSADBW called fewer times
 * sse2-1  --- bit-parallel counting and PSADBW
 * sse2-2  --- bit-parallel counting - PSADBW called fewer times (the same
 optimization as in ssse3-2)
 * ssse3-unrl --- ssse3-2 with inner loop unrolled 4 times
 * sse2-unrl --- ssse2-2 with inner loop unrolled 4 times

 compilation:
 $ gcc -O3 -Wall -pedantic -std=c99 ssse3_popcount.c

 Author: Wojciech Mu�a
 e-mail: wojciech_mula@poczta.onet.pl
 www:	http://0x80.pl/

 License: BSD

 initial release 24-05-2008, last update $Date$
 */
#include "print.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <xmmintrin.h>
#include <tmmintrin.h>
#include <emmintrin.h>

#ifdef DEBUG
#include <assert.h>
#endif

#ifdef ALIGN_DATA
#	define __aligned__ __attribute__((aligned(64)))
#else
#	define __aligned__
#endif

#define _MAX_LENGTH_ 320

uint8_t BASE_SHIFT[16] __aligned__ = { 0x0, 0x4, 0x8, 0xc, 0x2, 0x6, 0xa, 0xe,
		0x1, 0x5, 0x9, 0xd, 0x3, 0x7, 0xb, 0xf };

char MASKA_16[16] __aligned__ = { 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',
		'A', 'A', 'A', 'A', 'A', 'A', 'A' };

char MASKC_16[16] __aligned__ = { 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C',
		'C', 'C', 'C', 'C', 'C', 'C', 'C' };

char MASKG_16[16] __aligned__ = { 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G',
		'G', 'G', 'G', 'G', 'G', 'G', 'G' };

char MASKT_16[16] __aligned__ = { 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T', 'T',
		'T', 'T', 'T', 'T', 'T', 'T', 'T' };

uint8_t BIT_A_16[16] __aligned__ = { 0x00, 0x00, 0x00, 0x00, //A
		0x00, 0x00, 0x00, 0x00, //A
		0x00, 0x00, 0x00, 0x00, //A
		0x00, 0x00, 0x00, 0x00 //A
		};

uint8_t BIT_C_16[16] __aligned__ = { 0x55, 0x55, 0x55, 0x55, //C
		0x55, 0x55, 0x55, 0x55, //C
		0x55, 0x55, 0x55, 0x55, //C
		0x55, 0x55, 0x55, 0x55 //C
		};

uint8_t BIT_G_16[16] __aligned__ = { 0xaa, 0xaa, 0xaa, 0xaa, //G
		0xaa, 0xaa, 0xaa, 0xaa, //G
		0xaa, 0xaa, 0xaa, 0xaa, //G
		0xaa, 0xaa, 0xaa, 0xaa //G
		};

uint8_t BIT_T_16[16] __aligned__ = { 0xff, 0xff, 0xff, 0xff, //T
		0xff, 0xff, 0xff, 0xff, //T
		0xff, 0xff, 0xff, 0xff, //T
		0xff, 0xff, 0xff, 0xff  //T
		};

uint8_t LOC_MASK[64] = { 0xc0, 0xc0, 0xc0, 0xc0, //1
		0xc0, 0xc0, 0xc0, 0xc0, //1
		0xc0, 0xc0, 0xc0, 0xc0, //1
		0xc0, 0xc0, 0xc0, 0xc0, //1
		0x0c, 0x0c, 0x0c, 0x0c, //3
		0x0c, 0x0c, 0x0c, 0x0c, //3
		0x0c, 0x0c, 0x0c, 0x0c, //3
		0x0c, 0x0c, 0x0c, 0x0c, //3
		0x30, 0x30, 0x30, 0x30, //2
		0x30, 0x30, 0x30, 0x30, //2
		0x30, 0x30, 0x30, 0x30, //2
		0x30, 0x30, 0x30, 0x30, //2
		0x03, 0x03, 0x03, 0x03, //4
		0x03, 0x03, 0x03, 0x03, //4
		0x03, 0x03, 0x03, 0x03, //4
		0x03, 0x03, 0x03, 0x03 //4
		};

char read_t[_MAX_LENGTH_] __aligned__;
//char ref_t[_MAX_LENGTH_] __aligned__;

char A_filled_t[_MAX_LENGTH_] __aligned__;

uint8_t read_bit_t[_MAX_LENGTH_ / 8] __aligned__;
uint8_t ref_bit_t[_MAX_LENGTH_ / 8] __aligned__;

void c_convert2bit(char *str, int length, uint8_t *bits) {
	int i;
	int j;
	int k;

	for (j = 0; j < length * 2 / (8 * sizeof(bits[0])) + 1; j++)
		bits[j] = 0;

	for (i = 0; i < length; i++) {
		j = i * 2 / (sizeof(bits[0]) * 8);
		k = i * 2 % (sizeof(bits[0]) * 8);

		switch (str[i]) {
		case 'C':
			bits[j] += (1ULL << (sizeof(bits[0]) * 8 - 2)) >> k;
			break;
		case 'G':
			bits[j] += (2ULL << (sizeof(bits[0]) * 8 - 2)) >> k;
			break;
		case 'T':
			bits[j] += (3ULL << (sizeof(bits[0]) * 8 - 2)) >> k;
			break;
		default:
			break;
		}
		/*
		 int m;
		 for (m = 63; m >= 0; m--) {
		 //			cout << "m:" << m << " ";
		 if (temp & (1ULL << m) )
		 printf("1");
		 else
		 printf("0");
		 }
		 */
	}
}

void sse3_convert2bit(char *str, int length, uint8_t *bits) {

	__m128i *shift_hint = (__m128i *) BASE_SHIFT;
	__m128i *input_str;
	__m128i *output_str;

	__m128i temp;
	__m128i result;

	//loading comparison values
	__m128i *maskA = (__m128i *) MASKA_16;
	__m128i *maskC = (__m128i *) MASKC_16;
	__m128i *maskG = (__m128i *) MASKG_16;
	__m128i *maskT = (__m128i *) MASKT_16;
	__m128i *mask;

	int i, j;

	for (i = 0; i < length; i += 64) {
		for (j = 0; j < 64; j += 16) {
			input_str = (__m128i *) (str + i + j);
			output_str = (__m128i *) (A_filled_t + i + j);
			*output_str = _mm_shuffle_epi8(*input_str, *shift_hint);
		}

		//printf("After shifting 0: %s\n", A_filled_t);

		for (j = 8; j < 64; j += 32) {
			output_str = (__m128i *) (A_filled_t + i + j);
			temp = _mm_loadu_si128(output_str);
			temp = _mm_shuffle_epi32(temp, 0x4e);
			_mm_storeu_si128(output_str, temp);
		}

		//printf("After shifting 1: %s\n", A_filled_t);

		input_str = (__m128i *) (A_filled_t + i + 16);
		output_str = (__m128i *) (A_filled_t + i + 32);
		temp = *output_str;
		*output_str = *input_str;
		*input_str = temp;

		//printf("After shifting 2: %s\n", A_filled_t);

		for (j = 0; j < 64; j += 16) {
			output_str = (__m128i *) (A_filled_t + i + j);
			temp = _mm_load_si128(output_str);
			temp = _mm_shuffle_epi32(temp, 0xd8);
			_mm_store_si128(output_str, temp);
		}

		//printf("After shifting 3: %s\n", A_filled_t);

		for (j = 8; j < 64; j += 32) {
			output_str = (__m128i *) (A_filled_t + i + j);
			temp = _mm_loadu_si128(output_str);
			temp = _mm_shuffle_epi32(temp, 0x4e);
			_mm_storeu_si128(output_str, temp);
		}

		result = _mm_set1_epi32(0);
		__m128i* bit_idx = (__m128i*) (bits + (i * 2) / (8 * sizeof(bits[0]) ) );
		*bit_idx = _mm_set1_epi32(0);
		for (j = 0; j < 64; j += 16) {
			output_str = (__m128i *) (A_filled_t + i + j);
			temp = _mm_cmpeq_epi8(*maskC, *output_str);
			result = _mm_and_si128(temp, *((__m128i *) BIT_C_16));
			//print128_hex(result);

			temp = _mm_cmpeq_epi8(*maskG, *output_str);
			temp = _mm_and_si128(temp, *((__m128i *) BIT_G_16));
			result = _mm_or_si128(result, temp);
			//print128_hex(result);

			temp = _mm_cmpeq_epi8(*maskT, *output_str);
			temp = _mm_and_si128(temp, *((__m128i *) BIT_T_16));
			result = _mm_or_si128(result, temp);
			//print128_hex(result);

			mask = (__m128i*)(LOC_MASK + j);

			result = _mm_and_si128(*mask, result);
			//print128_hex(result);
			*bit_idx = _mm_or_si128(*bit_idx, result);
			//print128_bit(result);
			//print128_bit(*bit_idx);

		}
	}

	//printf("After shifting 4: %s\n", A_filled_t);

}

#define OPT_COUNT 3

char* functions[OPT_COUNT] = { "verify", "serial", "sse3" };

void help(const char* progname) {
	int i;
	printf("%s ", progname);

	printf("%s", functions[0]);
	for (i = 1; i < OPT_COUNT; i++)
		printf("|%s", functions[i]);

	printf(" length-count repeat-count\n");
	exit(1);
}

char verify(int index, void (*func)(char*, int, uint8_t*), char *str, int length,
		uint8_t* bits, uint8_t* bits_ref) {
	char failed = 0;
	(*func)(str, length, bits);
	int i;

	printf("%10s -> ", functions[index]);

	for (i = 0; i <= (length - 1) * 2 / (sizeof(bits[0]) * 8); i++) {
		if (bits[i] != bits_ref[i])
			failed = 1;

		int j;
		for (j = sizeof(bits[0]) * 8 - 1; j >= 0; j--) {
			if (bits[i] & 1ULL << j)
				printf("1");
			else
				printf("0");
		}
	}

	printf("\n");

	if (failed)
		printf("***FAILED!!!***\n");

	return failed;
}

/*
 char verify(int index, uint32_t (*func)(uint64_t*, int), uint64_t *buffer,
 int chunks8, unsigned popcount_ref) {
 char failed;
 uint32_t popcount = (*func)(buffer, chunks8);
 VERIFY(index);
 return failed;
 }
 */

int main(int argc, char* argv[]) {
	// prog parametrs
	int function;
	int length_count;
	int repeat_count;
	int default_length_count = 100;
	int default_repeat_count = 100000;

	double time_beg = 0;
	double time_end = 0;

	int i;

	// parse arguments
	if (argc == 1)
		help(argv[0]);
	// - function
	for (function = 0; function < OPT_COUNT; function++)
		if (strcasecmp(argv[1], functions[function]) == 0)
			break;

	if (function == OPT_COUNT)
		help(argv[0]);

	// - 16-byte chunks
	if (argc >= 3) {
		length_count = atoi(argv[2]);
		if (length_count <= 0 || length_count > _MAX_LENGTH_)
			help(argv[0]);
	} else
		length_count = default_length_count;

	// - repeat count
	if (argc >= 4) {
		repeat_count = atoi(argv[3]);
		if (repeat_count <= 0)
			help(argv[0]);
	} else
		repeat_count = default_repeat_count;

	// fill buffer with random data
//	srand(time(NULL ));
//	for (i = 0; i < sizeof(read_t); i++) {
//		read_t[i] = rand() % 4;
//		switch (read_t[i]) {
//		case 0:
//			read_t[i] = 'A';
//			break;
//		case 1:
//			read_t[i] = 'C';
//			break;
//		case 2:
//			read_t[i] = 'G';
//			break;
//		case 3:
//			read_t[i] = 'T';
//			break;
//		}
//	}

	strcpy(read_t,
			"ACGCTAGTAGCCGGAATAACAGGTAGGCCTACATTTTCTATACGGCGCCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");

	printf("Data: ");
	for (i = 0; i < length_count; i++)
		printf("%c", read_t[i]);
	printf("\n");

	// run
	printf("action=%s, length=%d, repeat count=%d\n", functions[function],
			length_count, repeat_count);

	char failed = 0;

	time_beg = clock();

	switch (function) {
	case 0:
		c_convert2bit(read_t, length_count, ref_bit_t);

		// serial is reference reference
		printf("%10s -> ", functions[1]);
		for (i = 0; i <= (length_count - 1) * 2 / (sizeof(ref_bit_t[0]) * 8); i++) {
			int j;
			for (j = sizeof(ref_bit_t[0]) * 8 - 1; j >= 0; j--) {
				if (ref_bit_t[i] & 1ULL << j)
					printf("1");
				else
					printf("0");
			}
		}
		printf("\n");

		failed |= verify(2, &sse3_convert2bit, read_t, length_count,
				read_bit_t, ref_bit_t);

		if (failed)
			return EXIT_FAILURE;

		break;

	case 1:
		while(repeat_count--)
			c_convert2bit(read_t, length_count, read_bit_t);
		break;

	case 2:
		while(repeat_count--)
			sse3_convert2bit(read_t, length_count, read_bit_t);
		break;

	}

	time_end = clock();

	printf("Total time: %f\n", (time_end - time_beg) / CLOCKS_PER_SEC);

	return EXIT_SUCCESS;
}

// eof
