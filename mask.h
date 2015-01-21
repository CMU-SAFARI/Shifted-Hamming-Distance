#ifndef __MASK_H__
#define __MASK_H__

#include <stdint.h>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/iteration.hpp>
#include <boost/preprocessor/arithmetic.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

#ifndef __aligned__
	#define __aligned__ __attribute__((aligned(16)))
#endif

#define SSE_BIT_LENGTH		128
#define SSE_BYTE_NUM		BOOST_PP_DIV(SSE_BIT_LENGTH, 8)

// For 1 bit bases
#define BASE_SIZE1			1
#define SSE_BASE_NUM1		BOOST_PP_DIV(SSE_BIT_LENGTH, BASE_SIZE1)
#define BYTE_BASE_NUM1		BOOST_PP_DIV(8, BASE_SIZE1)

// For 2 bit bases
#define BASE_SIZE11			2
#define SSE_BASE_NUM11		BOOST_PP_DIV(SSE_BIT_LENGTH, BASE_SIZE11)
#define BYTE_BASE_NUM11		BOOST_PP_DIV(8, BASE_SIZE11)

extern uint8_t *MASK_SSE_END1;
extern uint8_t *MASK_SSE_END11;
extern uint8_t *MASK_SSE_BEG1;
extern uint8_t *MASK_SSE_BEG11;

extern uint8_t *MASK_0F;
extern uint8_t *MASK_7F;

extern uint8_t *MASK_0TO1;

#endif // __MASK_H__
