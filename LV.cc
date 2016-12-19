#include "LV.h"
#include <cstdio>

int LV::count_ID_length_sse(int lane_idx, int start_pos) {
	int A_idx_offset = 0;
	int B_idx_offset = 0;

	if (lane_idx < mid_lane)
		A_idx_offset = mid_lane - lane_idx;
	else if (lane_idx > mid_lane) 
		B_idx_offset = lane_idx - mid_lane;

	while (start_pos < buffer_length && A[start_pos - A_idx_offset] == B[start_pos - B_idx_offset]) {
		start_pos++;
	}

	return start_pos;
}

LV::LV() {
	ED_t = 0;

	cur_ED = NULL;
	start = NULL;
	end = NULL;

	mid_lane = 0;
	total_lanes = 0;
}

LV::~LV() {
	if (total_lanes != 0) {
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

void LV::init(int ED_threshold) {
	if (total_lanes != 0)
		this->~LV();

	ED_t = ED_threshold;
	total_lanes = 2 * ED_t + 3;
	mid_lane = ED_t + 1;

	cur_ED = new int[total_lanes];
	ED_info = new ED_INFO[ED_t + 1];

	start = new int* [total_lanes];
	end = new int* [total_lanes];

	for (int i = 0; i < total_lanes; i++) {
		start[i] = new int [ED_t + 1]();
		end[i] = new int [ED_t + 1]();
	}
}

void LV::load_reads(char *read, char *ref, int length) {
	buffer_length = length;
	
	if (length > _MAX_LENGTH_)
		length = _MAX_LENGTH_;

	strncpy(A, read, length);
	strncpy(B, ref, length);

	//cout << "buffer_length: " << buffer_length << endl;
}

void LV::reset() {
	ED_pass = false;
	for (int i = 1; i < total_lanes - 1; i++) {
		int ED = abs(i - mid_lane);
		cur_ED[i] = ED;
		//start[i][ED] = ED;
		//end[i][ED] = ED;
	}
}

void LV::run() {
	end[mid_lane][0] = count_ID_length_sse(mid_lane, 0);
	cur_ED[mid_lane] = 1;

	if (end[mid_lane][0] == buffer_length) {
		final_lane_idx = mid_lane;
		final_ED = 0;
		ED_pass = true;
		return;
	}
	
	for (int e = 1; e <= ED_t; e++) {
		for (int l = 1; l < total_lanes - 1; l++) {
			if (cur_ED[l] == e) {
				
				//cout << "e: " << e << " l: " << l << endl;
				
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

				// Find the length of identical string
				start[l][e] = max_start;
				end[l][e] = count_ID_length_sse(l, max_start);

				//cout << "start[" << l << "][" << e << "]: " << start[l][e];
				//cout << "   end[" << l << "][" << e << "]: " << end[l][e] << endl;

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

bool LV::check_pass() {
	return ED_pass;
}

void LV::backtrack() {
	int lane_idx = final_lane_idx;
	int ED_probe = final_ED;

	while (ED_probe > abs(lane_idx - mid_lane) ) {
		int match_count = end[lane_idx][ED_probe] - start[lane_idx][ED_probe];
		ED_info[ED_probe].id_length = match_count;
		
		//cout << "start[" << lane_idx << "][" << ED_probe << "]: " << start[lane_idx][ED_probe];
		//cout << "   end[" << lane_idx << "][" << ED_probe - 1 << "]: " << end[lane_idx][ED_probe - 1] << endl;

		if (start[lane_idx][ED_probe] == (end[lane_idx][ED_probe - 1] + 1) ) {
			ED_info[ED_probe].type = MISMATCH;
		}
		else if (start[lane_idx][ED_probe] == end[lane_idx - 1][ED_probe - 1]) {
			lane_idx = lane_idx - 1;
			ED_info[ED_probe].type = A_INS;
		}
		else if (start[lane_idx][ED_probe] == end[lane_idx + 1][ED_probe - 1]) {
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

int LV::get_ED() {
	return final_ED;
}

string LV::get_CIGAR() {
	char buffer[32];
	string CIGAR;
	//CIGAR = to_string(ED_info[0].id_length);
	sprintf(buffer, "%d", ED_info[0].id_length);
	CIGAR = string(buffer);
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

		sprintf(buffer, "%d", ED_info[0].id_length);
		CIGAR += string(buffer);
		//CIGAR += to_string(ED_info[i].id_length);
	}

	return CIGAR;
}

