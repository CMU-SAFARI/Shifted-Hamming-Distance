/*
 * read_modifier.h
 *
 *  Created on: Nov 18, 2013
 *      Author: hxin
 */

#ifndef READ_MODIFIER_H_
#define READ_MODIFIER_H_

// Call this function to test fAlligner
unsigned long long test_alligner_random(int (*fAlligner)(char *, char *, int, int), char* DNA, int length, int testErr, int err, unsigned long long iterations);
unsigned long long test_alligner_random(int (*fAlligner0)(char *, char *, int, int), int (*fAlligner1)(char *, char *, int, int), char* DNA, int length, int testErr, int err);
unsigned long long test_alligner_random(int (*fAlligner)(char *, char *, int, int), char* DNA, int length, int testErr, int err);

unsigned long long test_alligner_exhaust_helper(int (*fAlligner)(char *, char *, int, int), char* refDNA, char* modDNA, int length, int testErr, int totErr, int err);
unsigned long long test_alligner_exhaust(int (*fAlligner)(char *, char *, int, int), char* DNA, int length, int testErr, int err);

void add_n_any(char* DNA, int length, int n);
void add_n_mis(char* DNA, int length, int n);
void add_n_ins(char* DNA, int length, int n);
void add_n_del(char* DNA, int length, int n);

void add_mis_pos_base(char* DNA, int length, int pos, char base);
void add_ins_pos_base(char* DNA, int length, int pos, char base);
void add_del_pos_base(char* DNA, int length, int pos, char base);

void add_mis_pos(char* DNA, int length, int pos);
void add_ins_pos(char* DNA, int length, int pos);
void add_del_pos(char* DNA, int length, int pos);

char get_rand_base();

#endif /* READ_MODIFIER_H_ */
