#include <iostream>
#include <x86intrin.h>
#include <emmintrin.h>
#include <immintrin.h>
#include <cstring>
#include <string>
#include <cstdlib>

#define _MAX_LENGTH_ 128

using namespace std;

#ifndef __ED_INFO_H_
#define __ED_INFO_H_

enum ED_TYPE {MISMATCH, A_INS, B_INS};

struct ED_INFO {
	ED_TYPE type;
	int id_length;	
};

#endif

#ifndef __LV_H_
#define __LV_H_

class LV {
public:
	LV();
	~LV();

	void init(int ED_threshold);

	void load_reads(char *read, char *ref, int length);

	void reset();	
	void run();
	bool check_pass();
	void backtrack();
	int get_ED();
	string get_CIGAR();
private:
	int count_ID_length_sse(int lane_idx, int start_pos);

	int ED_t;

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
	
	char A[_MAX_LENGTH_];
	char B[_MAX_LENGTH_];
};

#endif

