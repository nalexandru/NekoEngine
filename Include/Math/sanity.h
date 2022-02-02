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

	{ // vec4
		struct mat4 m;
		struct vec4 a, b, c;
		
		v4_add(&c, v4(&a, 1.f, 2.f, 3.f, 4.f), v4(&b, 1.f, 2.f, 3.f, 4.f));
		v4_log(&a, "a", MSMOD);
		v4_log(&b, "b", MSMOD);
		v4_log(&c, "v4_add (2, 4, 6, 8)", MSMOD);

		v4_sub(&c, v4(&a, 5.f, 5.f, 5.f, 5.f), v4(&b, 4.f, 3.f, 2.f, 1.f));
		v4_log(&c, "v4_sub (1, 2, 3, 4)", MSMOD);

		v4_mul(&c, v4(&a, 1.f, 2.f, 3.f, 4.f), v4(&b, 4.f, 3.f, 3.f, 2.f));
		v4_log(&c, "v4_mul (4, 6, 9, 8)", MSMOD);

		v4_div(&c, v4(&a, 10.f, 4.f, 9.f, 4.f), v4(&b, 2.f, 2.f, 3.f, 4.f));
		v4_log(&c, "v4_div (5, 2, 3, 1)", MSMOD);

		v4_adds(&c, v4(&a, 1.f, 2.f, 3.f, 4.f), 1.f);
		v4_log(&c, "v4_adds (2, 3, 4, 5)", MSMOD);

		v4_subs(&c, v4(&a, 5.f, 4.f, 3.f, 2.f), 1.f);
		v4_log(&c, "v4_subs (4, 3, 2, 1)", MSMOD);

		v4_muls(&c, v4(&a, 1.f, 2.f, 3.f, 4.f), 2.f);
		v4_log(&c, "v4_muls (2, 4, 6, 8)", MSMOD);

		v4_divs(&c, v4(&a, 10.f, 4.f, 8.f, 2.f), 2.f);
		v4_log(&c, "v4_divs (5, 2, 4, 1)", MSMOD);

		v4_norm(&c, v4(&a, 10.f, 4.f, 8.f, 2.f));
		v4_log(&c, "v4_norm (0.73721, 0.29488, 0.58977, 0.14744)", MSMOD);
		
		v4_scale(&c, v4(&a, 1.f, 2.f, 3.f, 4.f), 5.f);
		v4_log(&c, "v4_scale (0.91287, 1.82574, 2.73861, 3.65148)", MSMOD);
		
		v4_swap(v4(&a, 5.f, 6.f, 7.f, 8.f), v4(&b, 1.f, 2.f, 3.f, 4.f));
		v4_log(&a, "v4_swap_a (1, 2, 3, 4)", MSMOD);
		v4_log(&b, "v4_swap_b (5, 6, 7, 8)", MSMOD);
		
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_dot: %.02f", v4_dot(&a, &b));
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_len: %.02f", v4_len(&a));
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_len_sq: %.02f", v4_len_sq(&a));
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_distance: %.02f", v4_distance(&a, &b));
		
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_equal (true): %ls",
			v4_equal(v4(&a, 1.f, 1.f, 1.f, 1.f), v4(&b, 1.f, 1.f, 1.f, 1.f)) ? L"true" : L"false");
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_equal 6th (true): %ls",
			v4_equal(v4(&a, 1.f, 1.f, 1.f, .999999f), v4(&b, 1.f, 1.f, 1.f, .999999f)) ? L"true" : L"false");
			
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_equal (false): %ls",
			v4_equal(v4(&a, 1.f, 1.f, 1.f, 2.f), v4(&b, 1.f, 1.f, 1.f, 1.f)) ? L"true" : L"false");
		Sys_LogEntry(MSMOD, LOG_DEBUG, "v4_equal 6th (false): %ls",
			v4_equal(v4(&a, 1.f, 1.f, 1.f, .999999f), v4(&b, 1.f, 1.f, 1.f, .999998f)) ? L"true" : L"false");
		
		m4_perspective(&m, 45.f, 1280.f / 720.f, .1f, 100.f);
		
		v4_mul_m4(&c, v4_norm(&b, v4(&a, 10.f, 4.f, 8.f, 2.f)), &m);
		v4_log(&c, "v4_mul_m4 (2.08474, 1.48248, -0.73780, 0.08841)", MSMOD);
	}

	{ // mat4
		struct mat4 a, b, c, d, e, r;

		m4_perspective(&a, 45.f, 1280.f / 720.f, .1f, 100.f);
		m4_translate(&b, 10.f, 0.f, 1.f);

		m4_log(&a, "perspective", MSMOD);
		m4_log(&b, "translate", MSMOD);

		m4_mul(&r, &a, &b);
		m4_log(&a, "perspective (post mul)", MSMOD);
		m4_log(&b, "translate (post mul)", MSMOD);
		m4_log(&r, "m4_mul", MSMOD);

		m4_transpose(&r, &r);
		m4_log(&r, "m4_transpose", MSMOD);

		m4_inverse(&r, &r);
		m4_log(&r, "m4_inverse", MSMOD);

		m4_rot_x(&c, 20.f * M_PI_180);
		m4_mul(&d, &c, &b);
		m4_log(&c, "rotation", MSMOD);
		m4_log(&d, "modelview", MSMOD);

		m4_mul(&e, &a, &d);
		m4_log(&e, "mvp", MSMOD);
		
		m4_muls(&d, &e, 5.f);
		m4_log(&d, "m4_muls", MSMOD);
	}
}

#endif /* _NE_MATH_SANITY_H_ */
