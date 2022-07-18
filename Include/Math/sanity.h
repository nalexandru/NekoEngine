#ifndef _NE_MATH_SANITY_H_
#define _NE_MATH_SANITY_H_

#include <Math/Math.h>

#define MSMOD	"SanityTest"

static inline void
__MathDbg_SanityTest(void)
{
	Sys_LogEntry(MSMOD, LOG_DEBUG, "Instruction set: %ls",
#if defined(USE_AVX2)
		"AVX2"
#elif defined(USE_AVX)
		"AVX"
#elif defined(USE_SSE)
		"SSE"
#elif defined(USE_ALTIVEC)
		"Altivec"
#elif defined(USE_NEON)
		"NEON"
#elif defined(USE_VMX128)
		"VMX128"
#else
		"none"
#endif
	);

	{ // NeVec4
		struct NeMatrix m;
		struct NeVec4 a, b, c;
		
		M_AddVec4(&c, M_Vec4(&a, 1.f, 2.f, 3.f, 4.f), M_Vec4(&b, 1.f, 2.f, 3.f, 4.f));
		v4_log(&a, "a", MSMOD);
		v4_log(&b, "b", MSMOD);
		v4_log(&c, "M_AddVec4 (2, 4, 6, 8)", MSMOD);

		M_SubVec4(&c, M_Vec4(&a, 5.f, 5.f, 5.f, 5.f), M_Vec4(&b, 4.f, 3.f, 2.f, 1.f));
		v4_log(&c, "M_SubVec4 (1, 2, 3, 4)", MSMOD);

		M_MulVec4(&c, M_Vec4(&a, 1.f, 2.f, 3.f, 4.f), M_Vec4(&b, 4.f, 3.f, 3.f, 2.f));
		v4_log(&c, "M_MulVec4 (4, 6, 9, 8)", MSMOD);

		M_DivVec4(&c, M_Vec4(&a, 10.f, 4.f, 9.f, 4.f), M_Vec4(&b, 2.f, 2.f, 3.f, 4.f));
		v4_log(&c, "M_DivVec4 (5, 2, 3, 1)", MSMOD);

		M_AddVec4S(&c, M_Vec4(&a, 1.f, 2.f, 3.f, 4.f), 1.f);
		v4_log(&c, "M_AddVec4S (2, 3, 4, 5)", MSMOD);

		M_SubVec4S(&c, M_Vec4(&a, 5.f, 4.f, 3.f, 2.f), 1.f);
		v4_log(&c, "M_SubVec4S (4, 3, 2, 1)", MSMOD);

		M_MulVec4S(&c, M_Vec4(&a, 1.f, 2.f, 3.f, 4.f), 2.f);
		v4_log(&c, "M_MulVec4S (2, 4, 6, 8)", MSMOD);

		M_DivVec4S(&c, M_Vec4(&a, 10.f, 4.f, 8.f, 2.f), 2.f);
		v4_log(&c, "M_DivVec4S (5, 2, 4, 1)", MSMOD);

		M_NormalizeVec4(&c, M_Vec4(&a, 10.f, 4.f, 8.f, 2.f));
		v4_log(&c, "M_NormalizeVec4 (0.73721, 0.29488, 0.58977, 0.14744)", MSMOD);
		
		M_ScaleVec4(&c, M_Vec4(&a, 1.f, 2.f, 3.f, 4.f), 5.f);
		v4_log(&c, "M_ScaleVec4 (0.91287, 1.82574, 2.73861, 3.65148)", MSMOD);
		
		M_SwapVec4(M_Vec4(&a, 5.f, 6.f, 7.f, 8.f), M_Vec4(&b, 1.f, 2.f, 3.f, 4.f));
		v4_log(&a, "v4_swap_a (1, 2, 3, 4)", MSMOD);
		v4_log(&b, "v4_swap_b (5, 6, 7, 8)", MSMOD);
		
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_DotVec4: %.02f", M_DotVec4(&a, &b));
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_Vec4Length: %.02f", M_Vec4Length(&a));
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_Vec4LengthSquared: %.02f", M_Vec4LengthSquared(&a));
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_Vec4Distance: %.02f", M_Vec4Distance(&a, &b));
		
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_Vec4Equal (true): %ls",
			M_Vec4Equal(M_Vec4(&a, 1.f, 1.f, 1.f, 1.f), M_Vec4(&b, 1.f, 1.f, 1.f, 1.f)) ? L"true" : L"false");
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_Vec4Equal 6th (true): %ls",
			M_Vec4Equal(M_Vec4(&a, 1.f, 1.f, 1.f, .999999f), M_Vec4(&b, 1.f, 1.f, 1.f, .999999f)) ? L"true" : L"false");
			
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_Vec4Equal (false): %ls",
			M_Vec4Equal(M_Vec4(&a, 1.f, 1.f, 1.f, 2.f), M_Vec4(&b, 1.f, 1.f, 1.f, 1.f)) ? L"true" : L"false");
		Sys_LogEntry(MSMOD, LOG_DEBUG, "M_Vec4Equal 6th (false): %ls",
			M_Vec4Equal(M_Vec4(&a, 1.f, 1.f, 1.f, .999999f), M_Vec4(&b, 1.f, 1.f, 1.f, .999998f)) ? L"true" : L"false");
		
		M_PerspectiveMatrix(&m, 45.f, 1280.f / 720.f, .1f, 100.f);
		
		M_MulVec4Matrix(&c, M_NormalizeVec4(&b, M_Vec4(&a, 10.f, 4.f, 8.f, 2.f)), &m);
		v4_log(&c, "M_MulVec4Matrix (2.08474, 1.48248, -0.73780, 0.08841)", MSMOD);

		M_MulMatrixVec4(&c, M_NormalizeVec4(&b, M_Vec4(&a, 10.f, 4.f, 8.f, 2.f)), &m);
		v4_log(&c, "M_MulMatrixVec4 (2.08474, 1.48248, -0.73780, 0.08841)", MSMOD);
	}

	{ // NeMatrix
		struct NeMatrix a, b, c, d, e, r;

		M_PerspectiveMatrix(&a, 45.f, 1280.f / 720.f, .1f, 100.f);
		M_TranslationMatrix(&b, 10.f, 0.f, 1.f);

		M_LogMatrix(&a, "perspective", MSMOD);
		M_LogMatrix(&b, "translate", MSMOD);

		M_MulMatrix(&r, &a, &b);
		M_LogMatrix(&a, "perspective (post mul)", MSMOD);
		M_LogMatrix(&b, "translate (post mul)", MSMOD);
		M_LogMatrix(&r, "M_MulMatrix", MSMOD);

		M_TransposeMatrix(&r, &r);
		M_LogMatrix(&r, "M_TransposeMatrix", MSMOD);

		M_InverseMatrix(&r, &r);
		M_LogMatrix(&r, "M_InverseMatrix", MSMOD);

		M_RotationMatrixX(&c, 20.f * M_PI_180);
		M_MulMatrix(&d, &c, &b);
		M_LogMatrix(&c, "rotation", MSMOD);
		M_LogMatrix(&d, "modelview", MSMOD);

		M_MulMatrix(&e, &a, &d);
		M_LogMatrix(&e, "mvp", MSMOD);
		
		M_MulMatrixS(&d, &e, 5.f);
		M_LogMatrix(&d, "M_MulMatrixS", MSMOD);
	}
}

#endif /* _NE_MATH_SANITY_H_ */
