/*
 * bit_convertMain.c
 *
 *  Created on: Nov 7, 2013
 *      Author: hxin
 */
#include "bit_convert.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPT_COUNT 4

char* functions[OPT_COUNT] = { "verify", "serial", "sse3_11", "sse3_1" };

#define _MAX_LENGTH_ 320
#define _MAX_LENGTH_11_ 128

char read_t[_MAX_LENGTH_] __aligned__;

uint8_t read_bit_t[_MAX_LENGTH_ / 4] __aligned__;
uint8_t ref_bit_t[_MAX_LENGTH_ / 4] __aligned__;
uint8_t read_bit0_t[_MAX_LENGTH_ / 8] __aligned__;
uint8_t read_bit1_t[_MAX_LENGTH_ / 8] __aligned__;
uint8_t ref_bit0_t[_MAX_LENGTH_11_ / 8] __aligned__;
uint8_t ref_bit1_t[_MAX_LENGTH_11_ / 8] __aligned__;

void help(const char* progname) {
	int i;
	printf("%s ", progname);

	printf("%s", functions[0]);
	for (i = 1; i < OPT_COUNT; i++)
		printf("|%s", functions[i]);

	printf(" length-count repeat-count\n");
	exit(1);
}

char verify(int index, void (*func)(char*, int, uint8_t*), char *str,
		int length, uint8_t* bits, uint8_t* bits_ref) {
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

char verify(int index, void (*func)(char*, uint8_t*, uint8_t*), char *str,
		int length, uint8_t* bits0, uint8_t* bits1, uint8_t* bits_ref) {
	char failed = 0;
	(*func)(str, bits0, bits1);
	int i;

	uint8_t bit;

	printf("%10s -> ", functions[index]);

	for (i = 0; i <= (length - 1) * 2 / (sizeof(bits_ref[0]) * 8); i++) {
		bit = 0;

		if (i % 2 == 0) {
			for (int j = 0; j < 4; j++) {
				bit |= (bits0[i / 2] & (1 << j)) << j;
				bit |= (bits1[i / 2] & (1 << j)) << (j + 1);
			}
		} else {
			for (int j = 0; j < 4; j++) {
				bit |= (bits0[i / 2] & (1 << (j + 4))) >> 4 << j;
				bit |= (bits1[i / 2] & (1 << (j + 4))) >> 4 << (j + 1);
			}
		}

//		printf("bit: %x, bits_ref[%d]: %x\n", bit, i, bits_ref[i]);

		if (bit != bits_ref[i])
			failed = 1;

	}

//	printf("\nbit0: ");
//
//	for (i = 0; i < length / (sizeof(bits0[0]) * 8); i++) {
//		int j;
//		for (j = sizeof(bits0[0]) * 8 - 1; j >= 0; j--) {
//			if (bits0[i] & 1ULL << j)
//				printf("1");
//			else
//				printf("0");
//		}
//	}
//
//	printf("\nbit1: ");
//
//	for (i = 0; i < length / (sizeof(bits1[0]) * 8); i++) {
//		int j;
//		for (j = sizeof(bits1[0]) * 8 - 1; j >= 0; j--) {
//			if (bits1[i] & 1ULL << j)
//				printf("1");
//			else
//				printf("0");
//		}
//	}

//	printf("\nmerged together: ");

	for (i = 0; i < (length - 1) / (sizeof(bits1[0]) * 8) + 1; i++) {
		int j;
		for (j = 3; j >= 0; j--) {

			if (bits1[i] & 1ULL << j)
				printf("1");
			else
				printf("0");

			if (bits0[i] & 1ULL << j)
				printf("1");
			else
				printf("0");
		}

		for (j = 7; j >= 4; j--) {

			if (bits1[i] & 1ULL << j)
				printf("1");
			else
				printf("0");

			if (bits0[i] & 1ULL << j)
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
		for (i = 0; i <= (length_count - 1) * 2 / (sizeof(ref_bit_t[0]) * 8);
				i++) {
			int j;
			for (j = sizeof(ref_bit_t[0]) * 8 - 1; j >= 0; j--) {
				if (ref_bit_t[i] & 1ULL << j)
					printf("1");
				else
					printf("0");
			}
		}
		printf("\n");

//		for (i = 0; i <= (length_count - 1) * 2 / (sizeof(ref_bit_t[0]) * 8);
//				i++)
//			printf("%2x", ref_bit_t[i]);
//		printf("\n");

//		printf("Before swapping: %s\n", read_t);

		strcpy(read_t,
				"ACGCTAGTAGCCGGAATAACAGGTAGGCCTACATTTTCTATACGGCGCCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");
		failed |= verify(2, &sse3_convert2bit11, read_t, length_count,
				read_bit_t, ref_bit_t);
//		printf("After swapping: %s\n", read_t);

		strcpy(read_t,
				"ACGCTAGTAGCCGGAATAACAGGTAGGCCTACATTTTCTATACGGCGCCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");
		failed |= verify(3, &sse3_convert2bit1, read_t, length_count,
				read_bit0_t, read_bit1_t, ref_bit_t);
//		printf("After swapping: %s\n", read_t);

		if (failed)
			return EXIT_FAILURE;

		break;

	case 1:
		while (repeat_count--)
			c_convert2bit(read_t, length_count, read_bit_t);
		break;

	case 2:
		while (repeat_count--)
			sse3_convert2bit11(read_t, length_count, read_bit_t);
		break;

	case 3:
		while (repeat_count--)
			sse3_convert2bit1(read_t, read_bit0_t, read_bit1_t);
		break;

	}

	time_end = clock();

	printf("Total time: %f\n", (time_end - time_beg) / CLOCKS_PER_SEC);

	return EXIT_SUCCESS;
}

// eof

