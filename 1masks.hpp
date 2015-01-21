#define I		BOOST_PP_ITERATION()
#define PRINT_DATA(z, n, data) data

#define FF_NUM	BOOST_PP_DIV(I, BYTE_BASE_NUM1)
		BOOST_PP_ENUM(FF_NUM, PRINT_DATA, 0xff)

#if		FF_NUM != 0
		BOOST_PP_COMMA()
#endif	//FF_NUM != 0

#if		BOOST_PP_MOD(I, BYTE_BASE_NUM1) == 1
		0x01
#elif	BOOST_PP_MOD(I, BYTE_BASE_NUM1) == 2
		0x03
#elif	BOOST_PP_MOD(I, BYTE_BASE_NUM1) == 3
		0x07
#elif	BOOST_PP_MOD(I, BYTE_BASE_NUM1) == 4
		0x0f
#elif	BOOST_PP_MOD(I, BYTE_BASE_NUM1) == 5
		0x1f
#elif	BOOST_PP_MOD(I, BYTE_BASE_NUM1) == 6
		0x3f
#elif	BOOST_PP_MOD(I, BYTE_BASE_NUM1) == 7
		0x7f
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

