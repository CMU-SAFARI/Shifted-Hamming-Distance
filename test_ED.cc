#include "SIMD_ED.h"

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
	string CIGAR;
	int ED;
	test_obj.backtrack();
	cout << "ED: " << test_obj.get_ED() << " CIGAR: " << test_obj.get_CIGAR() << endl;

	return 0;
}
