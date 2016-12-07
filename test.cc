#include <iostream>
#include <x86intrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <cstring>
#include <string>
#include <cstdlib>
#include "bit_convert.h"
#include "vector_filter.h"
#include "print.h"

using namespace std;

#define _MAX_LENGTH_ 128

int count_ID_length_sse(__m128i bit_mask, int start_pos , int total_length) {
	__m128i shifted_mask = shift_left_sse1(bit_mask, start_pos);
	
	cout << "start_pos: " << start_pos << " ";
	print128_bit(shifted_mask);

	unsigned long *byte_cast = (unsigned long*) &shifted_mask;
	int length_result = 0;
	
	for (int i = 0; i < (total_length - start_pos) / 8 * sizeof(unsigned long); i++) {
		int id_length = _tzcnt_u64(byte_cast[i]);

		if (id_length == 0 && byte_cast[i] == 0) {
			id_length = 8 * sizeof(unsigned long);
			length_result += id_length;
		}
		else {
			length_result += id_length;
			break;
		}
	}

	cout << "length result: " << length_result << endl;

	if (length_result < total_length - start_pos)
		return length_result;
	else
		return total_length - start_pos;
}

class SIMD_ED {
public:
	SIMD_ED();
	~SIMD_ED();

	void init(int ED_threshold);

	void load_reads(char *read, char *ref, int length);

	void reset();	
	void run();
	void backtrack(int &ED, string &CIGAR);
private:
	int ED_t;
	__m128i *hamming_masks;

	// information of each lane
	int *cur_ED;
	int **start;
	int **end;

	// backtracking data
	bool ED_finished;
	int final_lane_idx;
	int final_ED;

	// serrogates
	int mid_lane;
	int total_lanes;
	
	// buffers
	int buffer_length;
	
	char A[_MAX_LENGTH_] __aligned__;
	char B[_MAX_LENGTH_] __aligned__;

	uint8_t A_bit0_t[_MAX_LENGTH_ / 4] __aligned__;
	uint8_t A_bit1_t[_MAX_LENGTH_ / 4] __aligned__;
	uint8_t B_bit0_t[_MAX_LENGTH_ / 4] __aligned__;
	uint8_t B_bit1_t[_MAX_LENGTH_ / 4] __aligned__;

};

SIMD_ED::SIMD_ED() {
	ED_t = 0;

	hamming_masks = NULL;

	cur_ED = NULL;
	start = NULL;
	end = NULL;

	mid_lane = 0;
	total_lanes = 0;
}

SIMD_ED::~SIMD_ED() {
	if (total_lanes != 0) {
		delete [] hamming_masks;
		delete [] cur_ED;

		for (int i = 0; i < total_lanes; i++) {
			delete [] start[i];
			delete [] end[i];
		}

		delete [] start;
		delete [] end;

		total_lanes = 0;
	}
}

void SIMD_ED::init(int ED_threshold) {
	if (total_lanes != 0)
		this->~SIMD_ED();

	ED_t = ED_threshold;
	total_lanes = 2 * ED_t + 3;
	mid_lane = ED_t + 1;

	hamming_masks = new __m128i [total_lanes];

	cur_ED = new int[total_lanes];

	start = new int* [total_lanes];
	end = new int* [total_lanes];

	for (int i = 0; i < total_lanes; i++) {
		start[i] = new int [ED_t]();
		end[i] = new int [ED_t]();
	}
}

void SIMD_ED::load_reads(char *read, char *ref, int length) {
	buffer_length = length;
	
	if (length > _MAX_LENGTH_)
		length = _MAX_LENGTH_;

	strncpy(A, read, length);

	sse3_convert2bit1(A, A_bit0_t, A_bit1_t);
	strncpy(B, ref, length);
	sse3_convert2bit1(B, B_bit0_t, B_bit1_t);

	cout << "A: " << A  << endl;
	cout << "B: " << B  << endl;

	__m128i *A0 = (__m128i*) A_bit0_t;
	__m128i *A1 = (__m128i*) A_bit1_t;
	__m128i *B0 = (__m128i*) B_bit0_t;
	__m128i *B1 = (__m128i*) B_bit1_t;

	for (int i = 1; i < total_lanes - 1; i++) {
		__m128i shifted_A0 = *A0;
		__m128i shifted_A1 = *A1;
		__m128i shifted_B0 = *B0;
		__m128i shifted_B1 = *B1;

		int shift_amount = abs(i - mid_lane);

		if (i < mid_lane) {
			shifted_A0 = shift_left_sse1(shifted_A0, shift_amount);
			shifted_A1 = shift_left_sse1(shifted_A1, shift_amount);
		}
		else if (i > mid_lane) {
			shifted_B0 = shift_left_sse1(shifted_B0, shift_amount);
			shifted_B1 = shift_left_sse1(shifted_B1, shift_amount);
		}

		__m128i mask_bit0 = _mm_xor_si128(shifted_A0, shifted_B0);
		__m128i mask_bit1 = _mm_xor_si128(shifted_A1, shifted_B1);

		hamming_masks[i] = _mm_or_si128(mask_bit0, mask_bit1);

		cout << "hamming_masks[" << i << "]: ";
		print128_bit(hamming_masks[i]);
		cout << endl;
	}
}

void SIMD_ED::reset() {
	ED_finished = false;
	for (int i = 1; i < total_lanes - 1; i++) {
		int ED = abs(i - mid_lane);
		cur_ED[i] = ED;
		start[i][ED] = ED;
		end[i][ED] = ED;
	}
}

void SIMD_ED::run() {
	int length = count_ID_length_sse(hamming_masks[mid_lane], 0, buffer_length);

	cout << "length: " << length << endl;

	end[mid_lane][0] = length;
	cur_ED[mid_lane] = 1;
	
	for (int e = 1; e < ED_t; e++) {
		for (int l = 1; l < total_lanes - 1; l++) {
			if (cur_ED[l] == e) {
				cout << "e: " << e << " l: " << l << endl;

				// Find the largest starting position
				int max_start = end[l][e-1] + 1;
				if (end[l-1][e-1] > max_start)
					max_start = end[l-1][e-1];
				if (end[l+1][e-1] > max_start)
					max_start = end[l+1][e-1];

				// Find the length of identical string
				length = count_ID_length_sse(hamming_masks[l], max_start, buffer_length);

				start[l][e] = max_start;
				end[l][e] = max_start + length;

				cout << "start[" << l << "][" << e << "]: " << start[l][e];
				cout << "   end[" << l << "][" << e << "]: " << end[l][e] << endl;

				if (end[l][e] == buffer_length) {
					final_lane_idx = l;
					final_ED = e;
					ED_finished = true;
					
					break;
				}

				cur_ED[l]++;
			}

		}

		if (ED_finished)
			break;
	}
}

int main () {
	unsigned int *test = new unsigned int[4];

	//for (int i = 0; i < 4; i++)
		//test[i] = 1 << i;
	test[0] = 1;
	test[1] = 0;
	test[2] = 0;
	test[3] = 0;


	//unsigned long int *bit_cast = (unsigned long int*) mask;
	unsigned long *bit_cast = (unsigned long*) test;

	for (int i = 0; i < 2; i++)
		cout << bit_cast[i] << " " <<  8 * sizeof(unsigned long) - _tzcnt_u64(bit_cast[i]) << endl;
	
	unsigned long checking[2];
	checking[0] = 0;
	checking[1] = 4;
	for (int i = 0; i < 2; i++)
		cout << 8 * sizeof(unsigned long) - _tzcnt_u64(checking[i]) << endl;


	char read_t[_MAX_LENGTH_] __aligned__;
	char ref_t[_MAX_LENGTH_] __aligned__;

	uint8_t read_bit0_t[_MAX_LENGTH_ / 4] __aligned__;
	uint8_t read_bit1_t[_MAX_LENGTH_ / 4] __aligned__;
	uint8_t ref_bit0_t[_MAX_LENGTH_ / 4] __aligned__;
	uint8_t ref_bit1_t[_MAX_LENGTH_ / 4] __aligned__;


	strcpy(read_t,
"ACGCTAGTAGCCGGAATAACAGGTAGGCCTACATTTTCTATACGGCGCCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");

	sse3_convert2bit1(read_t, read_bit0_t, read_bit1_t);
	
	strcpy(ref_t,
"CGCTATTAGCCGGAATAACAGGTAGGCCTACATTTTCTATACGGCGCCGGCAACCTTGAGGGGCCGTGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGGC");

	sse3_convert2bit1(ref_t, ref_bit0_t, ref_bit1_t);

	__m128i *read_bit0_mask = (__m128i*) read_bit0_t;
	__m128i *read_bit1_mask = (__m128i*) read_bit1_t;
	
	__m128i *ref_bit0_mask = (__m128i*) ref_bit0_t;
	__m128i *ref_bit1_mask = (__m128i*) ref_bit1_t;

	*read_bit0_mask = shift_left_sse1(*read_bit0_mask, 1);
	*read_bit1_mask = shift_left_sse1(*read_bit1_mask, 1);

	__m128i mask_bit0 = _mm_xor_si128(*read_bit0_mask, *ref_bit0_mask);
	__m128i mask_bit1 = _mm_xor_si128(*read_bit1_mask, *ref_bit1_mask);

	__m128i mask = _mm_or_si128(mask_bit0, mask_bit1);

	print128_bit(mask);
	cout << endl;

	bit_cast = (unsigned long*) &mask;
	for (int i = 0; i < 2; i++)
		cout <<  bit_cast[i] << " " << _tzcnt_u64(bit_cast[i]) << endl;

	strcpy(read_t,
"ACGCTAGTAGCCGGAATAACAGGTAGGCCTACATTTTCTATACGGCGCCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");
	strcpy(ref_t,
"CGCTATTAGCCGGAATAACAGGTAGGCCTACATTTTCTATACGGCGCCGGCAACCTTGAGGGGCCGTGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGGC");

	SIMD_ED test_obj;
	test_obj.init(4);
	test_obj.load_reads(read_t, ref_t, 128);
	test_obj.reset();
	test_obj.run();

	return 0;
}
