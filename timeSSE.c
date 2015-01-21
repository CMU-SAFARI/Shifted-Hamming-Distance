/*
 * timeSSE.c
 *
 *  Created on: Nov 14, 2013
 *      Author: hxin
 */
#include <stdio.h>
#include <emmintrin.h>

//uint8_t buff[32] __aligned__;

int main(int argc, char* argv[]) {
	int error = 0;
	unsigned long long repeat_count = 10000;
	int length = 64;

	if (argc >= 2)
		error = atoi(argv[1]);
	if (argc >= 3)
		repeat_count = strtoull(argv[2], NULL, 10);
	if (argc >= 4)
		length = atoi(argv[3]);

	repeat_count *= (unsigned long long) ((length - 1) / 64 + 1) * error;

	printf("error=%d, length=%d, repeat count=%llu\n", error, length,
			repeat_count);

	volatile __m128i a, b, c;

//	*curr = shift_right_sse(*prev, *curr, 1);
	while (repeat_count--) {
		c = _mm_xor_si128(a, b);
		c = _mm_and_si128(a, b);
	}
	return 0;
}

