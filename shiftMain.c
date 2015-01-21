/*
 * shiftMain.c
 *
 *  Created on: Nov 14, 2013
 *      Author: hxin
 */

#include "vector_filter.h"
#include "mask.h"
#include <stdio.h>

uint8_t buff[32] __aligned__;

int main(int argc, char* argv[]) {
	int shift_num = 0;
	unsigned long long repeat_count = 10000;
	int length = 64;

	if (argc >= 2)
		shift_num = atoi(argv[1]);
	if (argc >= 3)
		repeat_count = strtoull(argv[2], NULL, 10);
	if (argc >= 4)
		length = atoi(argv[3]);

	repeat_count *= (unsigned long long)( (length - 1) / 64 + 1);

	int i;
	for (i = 0; i < 16; i++)
		buff[i] = 0x0;
	for (i = 16; i < 32; i++)
		buff[i] = 0xff;
//	buff[15] = 0xc0;

	printf("length=%d, repeat count=%llu\n", length, repeat_count);

	printf("MASK_SSE_END:\n");
	for (i = 0; i < 128 * 16; i++)
		printf("%x, ", MASK_SSE_END1[i]);

//	printf("Data: ");
//	printbytevector(buff, 32);
//	printf("\n");

	__m128i *prev, *curr;

	prev = (__m128i *) buff;
	curr = (__m128i *) (buff + 16);

	__m128i zerofilled;
	zerofilled = _mm_set1_epi8(0x0);

//	*curr = shift_right_sse(*prev, *curr, 1);
	while (repeat_count--)
		shift_left_sse11(*prev, *curr, shift_num);

//	*curr = _mm_slli_epi16(*curr, 2);

//	printf("Data: ");
//	printbytevector(buff, 32);
//	printf("\n");
}
