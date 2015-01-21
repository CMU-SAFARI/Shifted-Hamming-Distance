/*
 * vector_filterMain.c
 *
 *  Created on: Nov 12, 2013
 *      Author: hxin
 */

//#ifndef BOOST_PP_IS_ITERATING
//#include "print.h"
#include <stdint.h>
#include "vector_filter.h"
#include <stdio.h>
#include <string.h>
#include <nmmintrin.h>
#include <emmintrin.h>

#include "mask.h"
/*
 #include <boost/preprocessor/repetition.hpp>
 #include <boost/preprocessor/iteration.hpp>
 #include <boost/preprocessor/arithmetic.hpp>
 #include <boost/preprocessor/punctuation/comma_if.hpp>

 #define SSE_BIT_LENGTH		128
 #define BASE_SIZE			2
 #define SSE_BASE_NUM		BOOST_PP_DIV(SSE_BIT_LENGTH, BASE_SIZE)
 #define BYTE_BASE_NUM		BOOST_PP_DIV(8, BASE_SIZE)
 #define SSE_BYTE_NUM		BOOST_PP_DIV(SSE_BIT_LENGTH, 8)

 uint8_t MASK_SSE_END[SSE_BIT_LENGTH * SSE_BYTE_NUM / BASE_SIZE] = {

 #define BOOST_PP_ITERATION_LIMITS	(0, SSE_BIT_LENGTH / 2  - 1)
 #define BOOST_PP_FILENAME_1			"vector_filterMain.c" // this file
 #include BOOST_PP_ITERATE()

 #else // BOOST_PP_IS_ITERATING

 #define I		BOOST_PP_ITERATION()
 #define PRINT_DATA(z, n, data) data

 #define FF_NUM	BOOST_PP_DIV(I, BYTE_BASE_NUM)
 BOOST_PP_ENUM(FF_NUM, PRINT_DATA, 0xff)

 #if		FF_NUM != 0
 BOOST_PP_COMMA()
 #endif	//FF_NUM != 0

 #if		BOOST_PP_MOD(I, BYTE_BASE_NUM) == 1
 0x03
 #elif	BOOST_PP_MOD(I, BYTE_BASE_NUM) == 2
 0x0f
 #elif	BOOST_PP_MOD(I, BYTE_BASE_NUM) == 3
 0x3f
 #else
 0x00
 #endif	//End of switch

 #define ZZ_NUM	BOOST_PP_SUB( BOOST_PP_SUB(SSE_BYTE_NUM, 1), FF_NUM)

 #if		ZZ_NUM != 0
 BOOST_PP_COMMA()
 #endif	//ZZ_NUM != 0

 BOOST_PP_ENUM(ZZ_NUM, PRINT_DATA, 0x00)

 #if I != BOOST_PP_ITERATION_FINISH()
 BOOST_PP_COMMA()
 #endif // I != BOOST_PP_ITERATION_FINISH()

 #undef	FF_NUM
 #undef	ZZ_NUM
 #undef	I
 #undef	PRINT_DATA

 #endif // BOOST_PP_IS_ITERATING
 #ifndef BOOST_PP_IS_ITERATING
 };

 */

#define _MAX_LENGTH_ 320

char read_t[_MAX_LENGTH_] __aligned__;
char ref_t[_MAX_LENGTH_] __aligned__;

//uint8_t read_bit_t[_MAX_LENGTH_ / 4] __aligned__;
//uint8_t ref_bit_t[_MAX_LENGTH_ / 4] __aligned__;

int main(int argc, char* argv[]) {

	int length = 128;
	int error = 0;
	int repeat_count = 10000;
	int average_loc = 10;

	strcpy(read_t,
			"AAAAAAAAAAAAAGACTAACCACCTTGTCCTGTTGTCTGTCTGGTCAGCCAATCATTGGGACCACACACCCCAGCATCGTGGACTGCGTGCTGAAGGTGC");
//			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCTACATCAGCTATACGGCATCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATG");
//			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCTCATCAGCTATACGGCTTCGGCAACCTTGAGGGGCCGCGCCCCGTTACCCTTTATACGTTTCCCGGGCAAGCCTTCGTGTGGGAGCATATGTATATGG");
//			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCTACATCAGCTATACGGCATCGGCAACCTTGAGTGGCCGCGGCCCGTTACACTTTATACGTTATCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATG");
	strcpy(ref_t,
			"AAAAAAAAAAAAAGACTAACCACCTTGTCCTGTTGTCTGTCTGGTCAGCCAATCATTGGGACCACACACCCCAGCATGGTGGACTGCGTGCTGAAGGGGC");
//			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCTACATCAGCTATACGGCCGTCGGCAACCTTGAGGGGTCGCGCCCCGTTACACTTTATACGTTTACCATTGCAAGCCTTCGTGTCGGAGCATATGTATA");
//			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCT ACATCAGCTATACGGCATCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATG");
//			"TCGCTAGTTAGCCGGACCTAAAGGTAGGCCTACATCAGCTATACGGCATCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTCTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");

	if (argc >= 2)
		length = atoi(argv[1]);
	if (argc >= 3)
		error = atoi(argv[2]);
	if (argc >= 4)
		repeat_count = atoi(argv[3]);
	if (argc >= 5)
		average_loc = atoi(argv[4]);

//	for (int i = 0; i < SSE_BIT_LENGTH * SSE_BYTE_NUM / BASE_SIZE1; i++) {
//		if (i % SSE_BYTE_NUM == 0)
//			printf("\n");
//		printf("%x ", MASK_SSE_END1[i]);
//	}

//	if (read_t[repeat_count] = 'A')
//		read_t[repeat_count] = 'C';
//	else
//		read_t[repeat_count] = 'A';

//		printf("\n");

//	while (repeat_count--)
//		bit_vec_filter_sse_simulate11(read_t, ref_t, length, error, average_loc);
//	while (repeat_count--)
//		bit_vec_filter_sse_simulate1(read_t, ref_t, length, error, average_loc);
//	if (bit_vec_filter_sse11(read_t, ref_t, length, error))
//		printf("Pass Filter\n");
//	else
//		printf("Fail Filter\n");
//
	
	if (bit_vec_filter_sse1(read_t, ref_t, length, error))
		printf("Pass Filter\n");
	else
		printf("Fail Filter\n");
	
//	strcpy(read_t,
//			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCTACATCAGCTATACGGCATCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATG");
////			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCTACATCAGCTATACGGCATCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");
//	strcpy(ref_t,
//			"TCGCTAGTAGCCGGAACTAACAGGTAGGCCTACATCAGCTATACGGCCGTCGGCAACCTTGAGGGGTCGCGCCCCGTTACACTTTATACGTTTACCATTGCAAGCCTTCGTGTCGGAGCATATGTATA");
////			"TCGCTAGTTAGCCGGACCTAAAGGTAGGCCTACATCAGCTATACGGCATCGGCAACCTTGAGGGGCCGCGCCCCGTTACACTTTATACGTTTCCCTTGCAAGCCTTCGTGTCGGAGCATATGTATATGG");
//	
//	if (bit_vec_filter_no_flipping_sse1(read_t, ref_t, length, error))
//		printf("Pass Filter\n");
//	else
//		printf("Fail Filter\n");

	return 0;

}

//#endif
