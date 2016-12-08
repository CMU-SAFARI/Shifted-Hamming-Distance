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

#define _MAX_LENGTH_ 128

#ifndef __SIMD_ED_H_
#define __SIMD_ED_H_

using namespace std;

enum ED_TYPE {MISMATCH, A_INS, B_INS};

struct ED_INFO {
	ED_TYPE type;
	int id_length;	
};

class SIMD_ED {
public:
	SIMD_ED();
	~SIMD_ED();

	void init(int ED_threshold);

	void load_reads(char *read, char *ref, int length);

	void reset();	
	void run();
	bool check_pass();
	void backtrack();
	int get_ED();
	string get_CIGAR();
private:
	int ED_t;
	__m128i *hamming_masks;

	// information of each lane
	int *cur_ED;
	int **start;
	int **end;

	// backtracking data
	bool ED_pass;
	int final_lane_idx;
	int final_ED;
	ED_INFO *ED_info;

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

#endif

