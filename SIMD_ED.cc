#include<cstdio>
#include "SIMD_ED.h"

int SIMD_ED::count_ID_length_sse(int lane_idx, int start_pos) {
	__m128i shifted_mask = shift_left_sse1(hamming_masks[lane_idx], start_pos);
	
#ifdef debug	
	cout << "start_pos: " << start_pos << " ";
	print128_bit(shifted_mask);
#endif

	//unsigned long *byte_cast = (unsigned long*) &shifted_mask;
	unsigned long byte_cast [2] __aligned__;
	_mm_store_si128((__m128i*) byte_cast, shifted_mask); 


	int length_result = 0;
	
	for (int i = 0; i <= (buffer_length - start_pos - 1) / (8 * sizeof(unsigned long) ); i++) {
		int id_length = _tzcnt_u64(byte_cast[i]);

		if (id_length == 8 * sizeof(unsigned long) && byte_cast[i] == 0) {
			//cout << "A" << endl;
			id_length = 8 * sizeof(unsigned long);
			length_result += id_length;
		}
		else {
			//cout << "B, byte_cast[" << i <<"]:" << byte_cast[i] << endl;
			length_result += id_length;
			break;
		}
	}

#ifdef debug	
	cout << "length result: " << length_result << endl;
#endif

	if (length_result < buffer_length - start_pos)
		return length_result;
	else
		return buffer_length - start_pos;
}

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
	ED_info = new ED_INFO[ED_t + 1];

	start = new int* [total_lanes];
	end = new int* [total_lanes];

	for (int i = 0; i < total_lanes; i++) {
		start[i] = new int [ED_t + 1]();
		end[i] = new int [ED_t + 1]();
	}
}

void SIMD_ED::convert_reads(char *read, char *ref, int length, uint8_t *A0, uint8_t *A1, uint8_t *B0, uint8_t *B1) {
	strncpy(A, read, length);
	sse3_convert2bit1(A, A_bit0_t, A_bit1_t);
	strncpy(B, ref, length);
	sse3_convert2bit1(B, B_bit0_t, B_bit1_t);

	memcpy(A0, A_bit0_t, _MAX_LENGTH_ / 8);
	memcpy(A1, A_bit1_t, _MAX_LENGTH_ / 8);
	memcpy(B0, B_bit0_t, _MAX_LENGTH_ / 8);
	memcpy(B1, B_bit1_t, _MAX_LENGTH_ / 8);
}

void SIMD_ED::load_reads(char *read, char *ref, int length) {
	buffer_length = length;
	
	if (length > _MAX_LENGTH_)
		length = _MAX_LENGTH_;

	strncpy(A, read, length);
	sse3_convert2bit1(A, A_bit0_t, A_bit1_t);
	strncpy(B, ref, length);
	sse3_convert2bit1(B, B_bit0_t, B_bit1_t);

	//cout << "A: " << A  << endl;
	//cout << "B: " << B  << endl;
}

void SIMD_ED::load_reads(uint8_t *A0, uint8_t *A1, uint8_t *B0, uint8_t *B1, int length) {
	buffer_length = length;
	memcpy(A_bit0_t, A0, length / 8);
	memcpy(A_bit1_t, A1, length / 8);
	memcpy(B_bit0_t, B0, length / 8);
	memcpy(B_bit1_t, B1, length / 8);
}

void SIMD_ED::calculate_masks() {
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
			shifted_B0 = shift_right_sse1(shifted_B0, shift_amount);
			shifted_B1 = shift_right_sse1(shifted_B1, shift_amount);
		}
		else if (i > mid_lane) {
			shifted_A0 = shift_right_sse1(shifted_A0, shift_amount);
			shifted_A1 = shift_right_sse1(shifted_A1, shift_amount);
		}

		__m128i mask_bit0 = _mm_xor_si128(shifted_A0, shifted_B0);
		__m128i mask_bit1 = _mm_xor_si128(shifted_A1, shifted_B1);

		hamming_masks[i] = _mm_or_si128(mask_bit0, mask_bit1);

		//cout << "hamming_masks[" << i << "]: ";
		//print128_bit(hamming_masks[i]);
		//cout << endl;
	}
}

void SIMD_ED::reset() {
	ED_pass = false;
	for (int i = 1; i < total_lanes - 1; i++) {
		int ED = abs(i - mid_lane);
		cur_ED[i] = ED;
		//start[i][ED] = ED;
		//end[i][ED] = ED;
	}
}

void SIMD_ED::run() {
	int length = count_ID_length_sse(mid_lane, 0);

#ifdef debug	
	cout << "length result: " << length << " buffer_length: " << buffer_length << endl;
#endif

	end[mid_lane][0] = length;
	cur_ED[mid_lane] = 1;

	if (length == buffer_length) {
		final_lane_idx = mid_lane;
		final_ED = 0;
		ED_pass = true;
		return;
	}
	
	for (int e = 1; e <= ED_t; e++) {
		for (int l = 1; l < total_lanes - 1; l++) {
			if (cur_ED[l] == e) {
				
#ifdef debug	
				cout << "e: " << e << " l: " << l << endl;
#endif

				int top_offset = 0;
				int bot_offset = 0;

				if (l >= mid_lane)
					top_offset = 1;
				if (l <= mid_lane)
					bot_offset = 1;

				// Find the largest starting position
				int max_start = end[l][e-1] + 1;
				if (end[l-1][e-1] + top_offset > max_start)
					max_start = end[l-1][e-1] + top_offset;
				if (end[l+1][e-1] + bot_offset > max_start)
					max_start = end[l+1][e-1] + bot_offset;

				start[l][e] = max_start;

				// Find the length of identical string
				length = count_ID_length_sse(l, start[l][e]);

				end[l][e] = max_start + length;

#ifdef debug	
				cout << "start[" << l << "][" << e << "]: " << start[l][e];
				cout << "   end[" << l << "][" << e << "]: " << end[l][e] << endl;
#endif

				if (end[l][e] == buffer_length) {
					final_lane_idx = l;
					final_ED = e;
					ED_pass = true;
					
					break;
				}

				cur_ED[l]++;
			}

		}

		if (ED_pass)
			break;
	}
}

bool SIMD_ED::check_pass() {
	return ED_pass;
}

void SIMD_ED::backtrack() {
	int lane_idx = final_lane_idx;
	int ED_probe = final_ED;

	while (start[lane_idx][ED_probe] != abs(lane_idx - mid_lane) ) {

#ifdef debug
		cout << "end[" << lane_idx << "][" << ED_probe  << "]: " << end[lane_idx][ED_probe];
		cout << "    start[" << lane_idx << "][" << ED_probe << "]: " << start[lane_idx][ED_probe] << endl;
#endif

		int match_count = end[lane_idx][ED_probe] - start[lane_idx][ED_probe];
		ED_info[ED_probe].id_length = match_count;

		int top_offset = 0;
		int bot_offset = 0;

		if (lane_idx >= mid_lane)
			top_offset = 1;
		if (lane_idx <= mid_lane)
			bot_offset = 1;

		if (start[lane_idx][ED_probe] == (end[lane_idx][ED_probe - 1] + 1) ) {
			ED_info[ED_probe].type = MISMATCH;
		}
		else if (start[lane_idx][ED_probe] == end[lane_idx - 1][ED_probe - 1] + top_offset) {
			lane_idx = lane_idx - 1;
			ED_info[ED_probe].type = A_INS;
		}
		else if (start[lane_idx][ED_probe] == end[lane_idx + 1][ED_probe - 1] + bot_offset) {
			lane_idx = lane_idx + 1;
			ED_info[ED_probe].type = B_INS;
		}
		else
			cerr << "Error! No lane!!" << endl;
		
		ED_probe--;
	}

	int match_count = end[lane_idx][ED_probe] - start[lane_idx][ED_probe];
	ED_info[ED_probe].id_length = match_count;

	if (lane_idx < mid_lane) {
		for (int i = mid_lane - lane_idx; i > 0; i--) {
			ED_info[ED_probe].type = B_INS;
			ED_info[ED_probe - 1].id_length = 0;
		}
	}
	else if (lane_idx > mid_lane) {
		for (int i = lane_idx - mid_lane; i > 0; i--) {
			ED_info[ED_probe].type = A_INS;
			ED_info[ED_probe - 1].id_length = 0;
		}
	}

}

int SIMD_ED::get_ED() {
	return final_ED;
}

string SIMD_ED::get_CIGAR() {
	//char buffer[32];
	string CIGAR;
	CIGAR = to_string(ED_info[0].id_length);
	//sprintf(buffer, "%d", ED_info[0].id_length);
	//CIGAR = string(buffer);
	for (int i = 1; i <= final_ED; i++) {
		switch (ED_info[i].type) {
		case MISMATCH:
			CIGAR += 'M';
			break;
		case A_INS:
			CIGAR += 'I';
			break;
		case B_INS:
			CIGAR += 'D';
			break;
		}

		//sprintf(buffer, "%d", ED_info[0].id_length);
		//CIGAR += string(buffer);
		CIGAR += to_string(ED_info[i].id_length);
	}

	return CIGAR;
}

