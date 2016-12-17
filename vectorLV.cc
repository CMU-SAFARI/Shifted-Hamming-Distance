/*
 * vector_filterMain.c
 *
 *  Created on: Nov 12, 2013
 *      Author: hxin
 */

//#ifndef BOOST_PP_IS_ITERATING
//#include "print.h"
#include <string>
#include <sys/times.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LV.h"

#define BATCH_RUN 1000000 

using namespace std;

#include "mask.h"

//char read[128];
//char ref[128];

char init_all_NULL[128] = "";

//char read_t[128] __aligned__;// = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
//char ref_t[128] __aligned__;// = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

int main(int argc, char* argv[]) {
	
	string *read_strs = new string [BATCH_RUN];
	string *ref_strs = new string [BATCH_RUN];
	bool *valid_buff = new bool [BATCH_RUN];

	if (argc != 2) {
		printf("Usage: $>bin error\n");
		exit(1);
	}

	int error = atoi(argv[1]);

	size_t lineLength;	
	size_t length;
	char* tempstr = NULL;

	long long unsigned int passNum = 0;
	long long unsigned int totalNum = 0;

	long long read_size;
	long long read_idx;
	bool stop = false;

	tms start_time;
	tms end_time;
	tms elp_time;

	elp_time.tms_stime = 0;
	elp_time.tms_utime = 0;
	elp_time.tms_cstime = 0;
	elp_time.tms_cutime = 0;

	LV ed_obj;
	ed_obj.init(error);

	do {
		//clear past result
//		strncpy(read, init_all_NULL, 128);
//		strncpy(ref, init_all_NULL, 128);
		
		for (read_size = 0; read_size < BATCH_RUN; read_size++) {
			
			//get read
			getline(&tempstr, &lineLength, stdin);
			length = strlen(tempstr);
			//Get rid of the new line character
			tempstr[length - 1] = '\0';
			
			if (strcmp(tempstr, "end_of_file\0") == 0) {
				stop = true;
				break;
			}
			read_strs[read_size].assign(tempstr);

			//get ref
			getline(&tempstr, &lineLength, stdin);
			length = strlen(tempstr);
			//Get rid of the new line character
			tempstr[length - 1] = '\0';
			ref_strs[read_size].assign(tempstr);
			valid_buff[read_size] = false;
		}

		times(&start_time);

		for (read_idx = 0; read_idx < read_size; read_idx++) {

			length = read_strs[read_idx].length();
			if (length > 128)
				length = 128;

			//cout << "length: " << length << endl;
			
			ed_obj.load_reads((char*) read_strs[read_idx].c_str(), (char*) ref_strs[read_idx].c_str(), length);
			ed_obj.reset();
			ed_obj.run();
			if (ed_obj.check_pass() ) {
				//ed_obj.backtrack();
				valid_buff[read_idx] = true;
			}
/*
			else {
				fprintf(stdout, "error!\n");
				fprintf(stdout, "%.*s\n", 128, read_strs[read_idx].c_str() );
				fprintf(stdout, "%.*s\n", 128, ref_strs[read_idx].c_str() );
				exit(1);
			}
*/
			/*			
			strncpy(read_t, init_all_NULL, 128);
			strncpy(ref_t, init_all_NULL, 128);

			length = read_strs[read_idx].length();

			if (length > 128)
				length = 128;
			strncpy(read_t, read_strs[read_idx].c_str(), length);

			length = ref_strs[read_idx].length();
			//Get rid of the new line character
			if (length > 128)
				length = 128;
			strncpy(ref_t, ref_strs[read_idx].c_str(), length);

			if (bit_vec_filter_sse1(read_t, ref_t, length, error))
				valid_buff[read_idx] = true;
			*/
		}

		times(&end_time);

		for (read_idx = 0; read_idx < read_size; read_idx++) {

			if (valid_buff[read_idx]) {
				fprintf(stderr, "%.*s\n", 128, read_strs[read_idx].c_str() );
				fprintf(stderr, "%.*s\n", 128, ref_strs[read_idx].c_str() );
				passNum++;
			}
			totalNum++;
		}

		
		elp_time.tms_stime += end_time.tms_stime - start_time.tms_stime;
		elp_time.tms_utime += end_time.tms_utime - start_time.tms_utime;

		if (stop)
			break;

	} while (1);

	fprintf(stderr, "end_of_file\n");
	printf("passNum:\t%lld\n", passNum);
	printf("totalNum:\t%lld\n", totalNum);
	printf("total_time: %f\n", (double) elp_time.tms_utime / sysconf(_SC_CLK_TCK) ); 

	delete [] read_strs;
	delete [] ref_strs;

	return 0;

}

//#endif
