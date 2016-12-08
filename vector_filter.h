/*
 * vector_filter.h
 *
 *  Created on: Nov 12, 2013
 *      Author: hxin
 */

#ifndef VECTOR_FILTER_H_
#define VECTOR_FILTER_H_

#ifndef __aligned__
#define __aligned__ __attribute__((aligned(16)))
#endif

#include <stdint.h>
#include <nmmintrin.h>

// read and ref need to be 16 aligned
void bit_vec_filter_sse_simulate1(char* read, char* ref, int length, int max_error, int loc_num);
void bit_vec_filter_no_flipping_sse_simulate1(char* read, char* ref, int length, int max_error, int loc_num);
void bit_vec_filter_sse_simulate11(char* read, char* ref, int length, int max_error, int loc_num);

int bit_vec_filter_sse1(char* read, char* ref, int length, int max_error);
int bit_vec_filter_no_flipping_sse1(char* read, char* ref, int length, int max_error);
int bit_vec_filter_no_flipping_m128_sse1(uint8_t *read_vec0, uint8_t *read_vec1, uint8_t *ref_vec0,
											uint8_t *ref_vec1, __m128i mask, int max_error);
int bit_vec_filter_no_flipping_m128_sse1(uint8_t *read_vec0, uint8_t *read_vec1, uint8_t *ref_vec0,
											uint8_t *ref_vec1, __m128i mask, int max_error);
int bit_vec_filter_m128_sse1(uint8_t *read_vec0, uint8_t *read_vec1, uint8_t *ref_vec0,
											uint8_t *ref_vec1, __m128i mask, int max_error);

int bit_vec_filter_sse11(char* read, char* ref, int length, int max_error);
int bit_vec_filter_m128_sse11(uint8_t *read_vec, uint8_t *ref_vec, int length, int max_error);

__m128i shift_right_sse1(__m128i pri_vec, int shift_num);
__m128i shift_left_sse1(__m128i vec, int shift_num);
__m128i shift_right_sse11(__m128i pri_vec, __m128i vec, int shift_num);
__m128i shift_left_sse11(__m128i vec, __m128i next_vec, int shift_num);

#endif /* VECTOR_FILTER_H_ */
