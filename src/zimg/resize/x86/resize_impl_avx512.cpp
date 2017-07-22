#if defined(_MSC_VER) && !defined(__INTEL_COMPILER)
  #undef ZIMG_X86_AVX512
#endif

#ifdef ZIMG_X86_AVX512

#include <algorithm>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <immintrin.h>
#include "common/align.h"
#include "common/ccdep.h"
#include "common/checked_int.h"
#include "common/except.h"
#include "common/make_unique.h"
#include "common/pixel.h"

#define HAVE_CPU_SSE
#define HAVE_CPU_SSE2
#define HAVE_CPU_AVX
#define HAVE_CPU_AVX2
  #include "common/x86/x86util.h"
#undef HAVE_CPU_SSE
#undef HAVE_CPU_SSE2
#undef HAVE_CPU_AVX
#undef HAVE_CPU_AVX2

#include "resize/resize_impl.h"
#include "resize_impl_x86.h"

namespace zimg {
namespace resize {

namespace {

static inline FORCE_INLINE void mm512_transpose16_ps(__m512 &row0, __m512 &row1, __m512 &row2, __m512 &row3,
                                                     __m512 &row4, __m512 &row5, __m512 &row6, __m512 &row7,
                                                     __m512 &row8, __m512 &row9, __m512 &row10, __m512 &row11,
                                                     __m512 &row12, __m512 &row13, __m512 &row14, __m512 &row15)
{
	__m512 t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
	__m512 tt0, tt1, tt2, tt3, tt4, tt5, tt6, tt7, tt8, tt9, tt10, tt11, tt12, tt13, tt14, tt15;

	t0 = _mm512_unpacklo_ps(row0, row1);
	t1 = _mm512_unpackhi_ps(row0, row1);
	t2 = _mm512_unpacklo_ps(row2, row3);
	t3 = _mm512_unpackhi_ps(row2, row3);
	t4 = _mm512_unpacklo_ps(row4, row5);
	t5 = _mm512_unpackhi_ps(row4, row5);
	t6 = _mm512_unpacklo_ps(row6, row7);
	t7 = _mm512_unpackhi_ps(row6, row7);
	t8 = _mm512_unpacklo_ps(row8, row9);
	t9 = _mm512_unpackhi_ps(row8, row9);
	t10 = _mm512_unpacklo_ps(row10, row11);
	t11 = _mm512_unpackhi_ps(row10, row11);
	t12 = _mm512_unpacklo_ps(row12, row13);
	t13 = _mm512_unpackhi_ps(row12, row13);
	t14 = _mm512_unpacklo_ps(row14, row15);
	t15 = _mm512_unpackhi_ps(row14, row15);

	tt0 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t0), _mm512_castps_pd(t2)));
	tt1 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t0), _mm512_castps_pd(t2)));
	tt2 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t1), _mm512_castps_pd(t3)));
	tt3 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t1), _mm512_castps_pd(t3)));
	tt4 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t4), _mm512_castps_pd(t6)));
	tt5 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t4), _mm512_castps_pd(t6)));
	tt6 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t5), _mm512_castps_pd(t7)));
	tt7 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t5), _mm512_castps_pd(t7)));
	tt8 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t8), _mm512_castps_pd(t10)));
	tt9 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t8), _mm512_castps_pd(t10)));
	tt10 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t9), _mm512_castps_pd(t11)));
	tt11 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t9), _mm512_castps_pd(t11)));
	tt12 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t12), _mm512_castps_pd(t14)));
	tt13 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t12), _mm512_castps_pd(t14)));
	tt14 = _mm512_castpd_ps(_mm512_unpacklo_pd(_mm512_castps_pd(t13), _mm512_castps_pd(t15)));
	tt15 = _mm512_castpd_ps(_mm512_unpackhi_pd(_mm512_castps_pd(t13), _mm512_castps_pd(t15)));

	t0 = _mm512_shuffle_f32x4(tt0, tt4, 0x88);
	t1 = _mm512_shuffle_f32x4(tt1, tt5, 0x88);
	t2 = _mm512_shuffle_f32x4(tt2, tt6, 0x88);
	t3 = _mm512_shuffle_f32x4(tt3, tt7, 0x88);
	t4 = _mm512_shuffle_f32x4(tt0, tt4, 0xdd);
	t5 = _mm512_shuffle_f32x4(tt1, tt5, 0xdd);
	t6 = _mm512_shuffle_f32x4(tt2, tt6, 0xdd);
	t7 = _mm512_shuffle_f32x4(tt3, tt7, 0xdd);
	t8 = _mm512_shuffle_f32x4(tt8, tt12, 0x88);
	t9 = _mm512_shuffle_f32x4(tt9, tt13, 0x88);
	t10 = _mm512_shuffle_f32x4(tt10, tt14, 0x88);
	t11 = _mm512_shuffle_f32x4(tt11, tt15, 0x88);
	t12 = _mm512_shuffle_f32x4(tt8, tt12, 0xdd);
	t13 = _mm512_shuffle_f32x4(tt9, tt13, 0xdd);
	t14 = _mm512_shuffle_f32x4(tt10, tt14, 0xdd);
	t15 = _mm512_shuffle_f32x4(tt11, tt15, 0xdd);

	row0 = _mm512_shuffle_f32x4(t0, t8, 0x88);
	row1 = _mm512_shuffle_f32x4(t1, t9, 0x88);
	row2 = _mm512_shuffle_f32x4(t2, t10, 0x88);
	row3 = _mm512_shuffle_f32x4(t3, t11, 0x88);
	row4 = _mm512_shuffle_f32x4(t4, t12, 0x88);
	row5 = _mm512_shuffle_f32x4(t5, t13, 0x88);
	row6 = _mm512_shuffle_f32x4(t6, t14, 0x88);
	row7 = _mm512_shuffle_f32x4(t7, t15, 0x88);
	row8 = _mm512_shuffle_f32x4(t0, t8, 0xdd);
	row9 = _mm512_shuffle_f32x4(t1, t9, 0xdd);
	row10 = _mm512_shuffle_f32x4(t2, t10, 0xdd);
	row11 = _mm512_shuffle_f32x4(t3, t11, 0xdd);
	row12 = _mm512_shuffle_f32x4(t4, t12, 0xdd);
	row13 = _mm512_shuffle_f32x4(t5, t13, 0xdd);
	row14 = _mm512_shuffle_f32x4(t6, t14, 0xdd);
	row15 = _mm512_shuffle_f32x4(t7, t15, 0xdd);
}

static inline FORCE_INLINE void mm512_transpose32_epi16(__m512i &row0, __m512i &row1, __m512i &row2, __m512i &row3, __m512i &row4, __m512i &row5, __m512i &row6, __m512i &row7,
                                                        __m512i &row8, __m512i &row9, __m512i &row10, __m512i &row11, __m512i &row12, __m512i &row13, __m512i &row14, __m512i &row15,
                                                        __m512i &row16, __m512i &row17, __m512i &row18, __m512i &row19, __m512i &row20, __m512i &row21, __m512i &row22, __m512i &row23,
                                                        __m512i &row24, __m512i &row25, __m512i &row26, __m512i &row27, __m512i &row28, __m512i &row29, __m512i &row30, __m512i &row31)
{
	__m512i t0, t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12, t13, t14, t15;
	__m512i t16, t17, t18, t19, t20, t21, t22, t23, t24, t25, t26, t27, t28, t29, t30, t31;
	__m512i tt0, tt1, tt2, tt3, tt4, tt5, tt6, tt7, tt8, tt9, tt10, tt11, tt12, tt13, tt14, tt15;
	__m512i tt16, tt17, tt18, tt19, tt20, tt21, tt22, tt23, tt24, tt25, tt26, tt27, tt28, tt29, tt30, tt31;

	t0 = _mm512_unpacklo_epi16(row0, row1);
	t1 = _mm512_unpacklo_epi16(row2, row3);
	t2 = _mm512_unpacklo_epi16(row4, row5);
	t3 = _mm512_unpacklo_epi16(row6, row7);
	t4 = _mm512_unpackhi_epi16(row0, row1);
	t5 = _mm512_unpackhi_epi16(row2, row3);
	t6 = _mm512_unpackhi_epi16(row4, row5);
	t7 = _mm512_unpackhi_epi16(row6, row7);

	tt0 = _mm512_unpacklo_epi32(t0, t1);
	tt1 = _mm512_unpackhi_epi32(t0, t1);
	tt2 = _mm512_unpacklo_epi32(t2, t3);
	tt3 = _mm512_unpackhi_epi32(t2, t3);
	tt4 = _mm512_unpacklo_epi32(t4, t5);
	tt5 = _mm512_unpackhi_epi32(t4, t5);
	tt6 = _mm512_unpacklo_epi32(t6, t7);
	tt7 = _mm512_unpackhi_epi32(t6, t7);

	t0 = _mm512_unpacklo_epi64(tt0, tt2);
	t1 = _mm512_unpackhi_epi64(tt0, tt2);
	t2 = _mm512_unpacklo_epi64(tt1, tt3);
	t3 = _mm512_unpackhi_epi64(tt1, tt3);
	t4 = _mm512_unpacklo_epi64(tt4, tt6);
	t5 = _mm512_unpackhi_epi64(tt4, tt6);
	t6 = _mm512_unpacklo_epi64(tt5, tt7);
	t7 = _mm512_unpackhi_epi64(tt5, tt7);

	t8 = _mm512_unpacklo_epi16(row8, row9);
	t9 = _mm512_unpacklo_epi16(row10, row11);
	t10 = _mm512_unpacklo_epi16(row12, row13);
	t11 = _mm512_unpacklo_epi16(row14, row15);
	t12 = _mm512_unpackhi_epi16(row8, row9);
	t13 = _mm512_unpackhi_epi16(row10, row11);
	t14 = _mm512_unpackhi_epi16(row12, row13);
	t15 = _mm512_unpackhi_epi16(row14, row15);

	tt8 = _mm512_unpacklo_epi32(t8, t9);
	tt9 = _mm512_unpackhi_epi32(t8, t9);
	tt10 = _mm512_unpacklo_epi32(t10, t11);
	tt11 = _mm512_unpackhi_epi32(t10, t11);
	tt12 = _mm512_unpacklo_epi32(t12, t13);
	tt13 = _mm512_unpackhi_epi32(t12, t13);
	tt14 = _mm512_unpacklo_epi32(t14, t15);
	tt15 = _mm512_unpackhi_epi32(t14, t15);

	t8 = _mm512_unpacklo_epi64(tt8, tt10);
	t9 = _mm512_unpackhi_epi64(tt8, tt10);
	t10 = _mm512_unpacklo_epi64(tt9, tt11);
	t11 = _mm512_unpackhi_epi64(tt9, tt11);
	t12 = _mm512_unpacklo_epi64(tt12, tt14);
	t13 = _mm512_unpackhi_epi64(tt12, tt14);
	t14 = _mm512_unpacklo_epi64(tt13, tt15);
	t15 = _mm512_unpackhi_epi64(tt13, tt15);

	tt0 = _mm512_shuffle_i32x4(t0, t8, 0x88);
	tt1 = _mm512_shuffle_i32x4(t1, t9, 0x88);
	tt2 = _mm512_shuffle_i32x4(t2, t10, 0x88);
	tt3 = _mm512_shuffle_i32x4(t3, t11, 0x88);
	tt4 = _mm512_shuffle_i32x4(t4, t12, 0x88);
	tt5 = _mm512_shuffle_i32x4(t5, t13, 0x88);
	tt6 = _mm512_shuffle_i32x4(t6, t14, 0x88);
	tt7 = _mm512_shuffle_i32x4(t7, t15, 0x88);

	tt8 = _mm512_shuffle_i32x4(t0, t8, 0xdd);
	tt9 = _mm512_shuffle_i32x4(t1, t9, 0xdd);
	tt10 = _mm512_shuffle_i32x4(t2, t10, 0xdd);
	tt11 = _mm512_shuffle_i32x4(t3, t11, 0xdd);
	tt12 = _mm512_shuffle_i32x4(t4, t12, 0xdd);
	tt13 = _mm512_shuffle_i32x4(t5, t13, 0xdd);
	tt14 = _mm512_shuffle_i32x4(t6, t14, 0xdd);
	tt15 = _mm512_shuffle_i32x4(t7, t15, 0xdd);

	t16 = _mm512_unpacklo_epi16(row16, row17);
	t17 = _mm512_unpacklo_epi16(row18, row19);
	t18 = _mm512_unpacklo_epi16(row20, row21);
	t19 = _mm512_unpacklo_epi16(row22, row23);
	t20 = _mm512_unpackhi_epi16(row16, row17);
	t21 = _mm512_unpackhi_epi16(row18, row19);
	t22 = _mm512_unpackhi_epi16(row20, row21);
	t23 = _mm512_unpackhi_epi16(row22, row23);

	tt16 = _mm512_unpacklo_epi32(t16, t17);
	tt17 = _mm512_unpackhi_epi32(t16, t17);
	tt18 = _mm512_unpacklo_epi32(t18, t19);
	tt19 = _mm512_unpackhi_epi32(t18, t19);
	tt20 = _mm512_unpacklo_epi32(t20, t21);
	tt21 = _mm512_unpackhi_epi32(t20, t21);
	tt22 = _mm512_unpacklo_epi32(t22, t23);
	tt23 = _mm512_unpackhi_epi32(t22, t23);

	t16 = _mm512_unpacklo_epi64(tt16, tt18);
	t17 = _mm512_unpackhi_epi64(tt16, tt18);
	t18 = _mm512_unpacklo_epi64(tt17, tt19);
	t19 = _mm512_unpackhi_epi64(tt17, tt19);
	t20 = _mm512_unpacklo_epi64(tt20, tt22);
	t21 = _mm512_unpackhi_epi64(tt20, tt22);
	t22 = _mm512_unpacklo_epi64(tt21, tt23);
	t23 = _mm512_unpackhi_epi64(tt21, tt23);

	t24 = _mm512_unpacklo_epi16(row24, row25);
	t25 = _mm512_unpacklo_epi16(row26, row27);
	t26 = _mm512_unpacklo_epi16(row28, row29);
	t27 = _mm512_unpacklo_epi16(row30, row31);
	t28 = _mm512_unpackhi_epi16(row24, row25);
	t29 = _mm512_unpackhi_epi16(row26, row27);
	t30 = _mm512_unpackhi_epi16(row28, row29);
	t31 = _mm512_unpackhi_epi16(row30, row31);

	tt24 = _mm512_unpacklo_epi32(t24, t25);
	tt25 = _mm512_unpackhi_epi32(t24, t25);
	tt26 = _mm512_unpacklo_epi32(t26, t27);
	tt27 = _mm512_unpackhi_epi32(t26, t27);
	tt28 = _mm512_unpacklo_epi32(t28, t29);
	tt29 = _mm512_unpackhi_epi32(t28, t29);
	tt30 = _mm512_unpacklo_epi32(t30, t31);
	tt31 = _mm512_unpackhi_epi32(t30, t31);

	t24 = _mm512_unpacklo_epi64(tt24, tt26);
	t25 = _mm512_unpackhi_epi64(tt24, tt26);
	t26 = _mm512_unpacklo_epi64(tt25, tt27);
	t27 = _mm512_unpackhi_epi64(tt25, tt27);
	t28 = _mm512_unpacklo_epi64(tt28, tt30);
	t29 = _mm512_unpackhi_epi64(tt28, tt30);
	t30 = _mm512_unpacklo_epi64(tt29, tt31);
	t31 = _mm512_unpackhi_epi64(tt29, tt31);

	tt16 = _mm512_shuffle_i32x4(t16, t24, 0x88);
	tt17 = _mm512_shuffle_i32x4(t17, t25, 0x88);
	tt18 = _mm512_shuffle_i32x4(t18, t26, 0x88);
	tt19 = _mm512_shuffle_i32x4(t19, t27, 0x88);
	tt20 = _mm512_shuffle_i32x4(t20, t28, 0x88);
	tt21 = _mm512_shuffle_i32x4(t21, t29, 0x88);
	tt22 = _mm512_shuffle_i32x4(t22, t30, 0x88);
	tt23 = _mm512_shuffle_i32x4(t23, t31, 0x88);

	tt24 = _mm512_shuffle_i32x4(t16, t24, 0xdd);
	tt25 = _mm512_shuffle_i32x4(t17, t25, 0xdd);
	tt26 = _mm512_shuffle_i32x4(t18, t26, 0xdd);
	tt27 = _mm512_shuffle_i32x4(t19, t27, 0xdd);
	tt28 = _mm512_shuffle_i32x4(t20, t28, 0xdd);
	tt29 = _mm512_shuffle_i32x4(t21, t29, 0xdd);
	tt30 = _mm512_shuffle_i32x4(t22, t30, 0xdd);
	tt31 = _mm512_shuffle_i32x4(t23, t31, 0xdd);

	row0 = _mm512_shuffle_i32x4(tt0, tt16, 0x88);
	row1 = _mm512_shuffle_i32x4(tt1, tt17, 0x88);
	row2 = _mm512_shuffle_i32x4(tt2, tt18, 0x88);
	row3 = _mm512_shuffle_i32x4(tt3, tt19, 0x88);
	row4 = _mm512_shuffle_i32x4(tt4, tt20, 0x88);
	row5 = _mm512_shuffle_i32x4(tt5, tt21, 0x88);
	row6 = _mm512_shuffle_i32x4(tt6, tt22, 0x88);
	row7 = _mm512_shuffle_i32x4(tt7, tt23, 0x88);

	row8 = _mm512_shuffle_i32x4(tt8, tt24, 0x88);
	row9 = _mm512_shuffle_i32x4(tt9, tt25, 0x88);
	row10 = _mm512_shuffle_i32x4(tt10, tt26, 0x88);
	row11 = _mm512_shuffle_i32x4(tt11, tt27, 0x88);
	row12 = _mm512_shuffle_i32x4(tt12, tt28, 0x88);
	row13 = _mm512_shuffle_i32x4(tt13, tt29, 0x88);
	row14 = _mm512_shuffle_i32x4(tt14, tt30, 0x88);
	row15 = _mm512_shuffle_i32x4(tt15, tt31, 0x88);

	row16 = _mm512_shuffle_i32x4(tt0, tt16, 0xdd);
	row17 = _mm512_shuffle_i32x4(tt1, tt17, 0xdd);
	row18 = _mm512_shuffle_i32x4(tt2, tt18, 0xdd);
	row19 = _mm512_shuffle_i32x4(tt3, tt19, 0xdd);
	row20 = _mm512_shuffle_i32x4(tt4, tt20, 0xdd);
	row21 = _mm512_shuffle_i32x4(tt5, tt21, 0xdd);
	row22 = _mm512_shuffle_i32x4(tt6, tt22, 0xdd);
	row23 = _mm512_shuffle_i32x4(tt7, tt23, 0xdd);

	row24 = _mm512_shuffle_i32x4(tt8, tt24, 0xdd);
	row25 = _mm512_shuffle_i32x4(tt9, tt25, 0xdd);
	row26 = _mm512_shuffle_i32x4(tt10, tt26, 0xdd);
	row27 = _mm512_shuffle_i32x4(tt11, tt27, 0xdd);
	row28 = _mm512_shuffle_i32x4(tt12, tt28, 0xdd);
	row29 = _mm512_shuffle_i32x4(tt13, tt29, 0xdd);
	row30 = _mm512_shuffle_i32x4(tt14, tt30, 0xdd);
	row31 = _mm512_shuffle_i32x4(tt15, tt31, 0xdd);
}


struct f16_traits {
	typedef __m256i vec16_type;
	typedef uint16_t pixel_type;

	static constexpr PixelType type_constant = PixelType::HALF;

	static inline FORCE_INLINE vec16_type load16_raw(const pixel_type *ptr)
	{
		return _mm256_load_si256((const __m256i *)ptr);
	}

	static inline FORCE_INLINE void store16_raw(pixel_type *ptr, vec16_type x)
	{
		_mm256_store_si256((__m256i *)ptr, x);
	}

	static inline FORCE_INLINE __m512 load16(const pixel_type *ptr)
	{
		return _mm512_cvtph_ps(_mm256_load_si256((const __m256i *)ptr));
	}

	static inline FORCE_INLINE void store16(pixel_type *ptr, __m512 x)
	{
		_mm256_store_si256((__m256i *)ptr, _mm512_cvtps_ph(x, 0));
	}

	static inline FORCE_INLINE void mask_store16(pixel_type *ptr, __mmask16 mask, __m512 x)
	{
		_mm256_mask_storeu_epi16((__m256i *)ptr, mask, _mm512_cvtps_ph(x, 0));
	}

	static inline FORCE_INLINE void transpose16(vec16_type &x0, vec16_type &x1, vec16_type &x2, vec16_type &x3,
	                                            vec16_type &x4, vec16_type &x5, vec16_type &x6, vec16_type &x7,
	                                            vec16_type &x8, vec16_type &x9, vec16_type &x10, vec16_type &x11,
	                                            vec16_type &x12, vec16_type &x13, vec16_type &x14, vec16_type &x15)
	{
		mm256_transpose16_epi16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
	}

	static inline FORCE_INLINE void scatter16(pixel_type *dst0, pixel_type *dst1, pixel_type *dst2, pixel_type *dst3,
	                                          pixel_type *dst4, pixel_type *dst5, pixel_type *dst6, pixel_type *dst7,
	                                          pixel_type *dst8, pixel_type *dst9, pixel_type *dst10, pixel_type *dst11,
	                                          pixel_type *dst12, pixel_type *dst13, pixel_type *dst14, pixel_type *dst15, __m512 x)
	{
		__m256i y = _mm512_cvtps_ph(x, 0);
		mm_scatter_epi16(dst0, dst1, dst2, dst3, dst4, dst5, dst6, dst7, _mm256_castsi256_si128(y));
		mm_scatter_epi16(dst8, dst9, dst10, dst11, dst12, dst13, dst14, dst15, _mm256_extracti128_si256(y, 1));
	}
};

struct f32_traits {
	typedef __m512 vec16_type;
	typedef float pixel_type;

	static constexpr PixelType type_constant = PixelType::FLOAT;

	static inline FORCE_INLINE vec16_type load16_raw(const pixel_type *ptr)
	{
		return _mm512_load_ps(ptr);
	}

	static inline FORCE_INLINE void store16_raw(pixel_type *ptr, vec16_type x)
	{
		_mm512_store_ps(ptr, x);
	}

	static inline FORCE_INLINE __m512 load16(const pixel_type *ptr)
	{
		return _mm512_load_ps(ptr);
	}

	static inline FORCE_INLINE void store16(pixel_type *ptr, __m512 x)
	{
		_mm512_store_ps(ptr, x);
	}

	static inline FORCE_INLINE void mask_store16(pixel_type *ptr, __mmask16 mask, __m512 x)
	{
		_mm512_mask_store_ps(ptr, mask, x);
	}

	static inline FORCE_INLINE void transpose16(vec16_type &x0, vec16_type &x1, vec16_type &x2, vec16_type &x3,
	                                            vec16_type &x4, vec16_type &x5, vec16_type &x6, vec16_type &x7,
	                                            vec16_type &x8, vec16_type &x9, vec16_type &x10, vec16_type &x11,
	                                            vec16_type &x12, vec16_type &x13, vec16_type &x14, vec16_type &x15)
	{
		mm512_transpose16_ps(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);
	}

	static inline FORCE_INLINE void scatter16(pixel_type *dst0, pixel_type *dst1, pixel_type *dst2, pixel_type *dst3,
	                                          pixel_type *dst4, pixel_type *dst5, pixel_type *dst6, pixel_type *dst7,
	                                          pixel_type *dst8, pixel_type *dst9, pixel_type *dst10, pixel_type *dst11,
	                                          pixel_type *dst12, pixel_type *dst13, pixel_type *dst14, pixel_type *dst15, __m512 x)
	{
		mm_scatter_ps(dst0, dst1, dst2, dst3, _mm512_extractf32x4_ps(x, 0));
		mm_scatter_ps(dst4, dst5, dst6, dst7, _mm512_extractf32x4_ps(x, 1));
		mm_scatter_ps(dst8, dst9, dst10, dst11, _mm512_extractf32x4_ps(x, 2));
		mm_scatter_ps(dst12, dst13, dst14, dst15, _mm512_extractf32x4_ps(x, 3));
	}
};


inline FORCE_INLINE __m512i export_i30_u16(__m512i lo, __m512i hi, uint16_t limit)
{
	const __m512i round = _mm512_set1_epi32(1 << 13);

	lo = _mm512_add_epi32(lo, round);
	hi = _mm512_add_epi32(hi, round);

	lo = _mm512_srai_epi32(lo, 14);
	hi = _mm512_srai_epi32(hi, 14);

	lo = _mm512_packs_epi32(lo, hi);

	return lo;
}


template <class Traits, class T>
void transpose_line_16x16(T *dst, const T *src_p[16], unsigned left, unsigned right)
{
	typedef typename Traits::vec16_type vec16_type;

	const T *src_p0 = src_p[0];
	const T *src_p1 = src_p[1];
	const T *src_p2 = src_p[2];
	const T *src_p3 = src_p[3];
	const T *src_p4 = src_p[4];
	const T *src_p5 = src_p[5];
	const T *src_p6 = src_p[6];
	const T *src_p7 = src_p[7];
	const T *src_p8 = src_p[8];
	const T *src_p9 = src_p[9];
	const T *src_p10 = src_p[10];
	const T *src_p11 = src_p[11];
	const T *src_p12 = src_p[12];
	const T *src_p13 = src_p[13];
	const T *src_p14 = src_p[14];
	const T *src_p15 = src_p[15];

	for (unsigned j = left; j < right; j += 16) {
		vec16_type x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;

		x0 = Traits::load16_raw(src_p0 + j);
		x1 = Traits::load16_raw(src_p1 + j);
		x2 = Traits::load16_raw(src_p2 + j);
		x3 = Traits::load16_raw(src_p3 + j);
		x4 = Traits::load16_raw(src_p4 + j);
		x5 = Traits::load16_raw(src_p5 + j);
		x6 = Traits::load16_raw(src_p6 + j);
		x7 = Traits::load16_raw(src_p7 + j);
		x8 = Traits::load16_raw(src_p8 + j);
		x9 = Traits::load16_raw(src_p9 + j);
		x10 = Traits::load16_raw(src_p10 + j);
		x11 = Traits::load16_raw(src_p11 + j);
		x12 = Traits::load16_raw(src_p12 + j);
		x13 = Traits::load16_raw(src_p13 + j);
		x14 = Traits::load16_raw(src_p14 + j);
		x15 = Traits::load16_raw(src_p15 + j);

		Traits::transpose16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);

		Traits::store16_raw(dst + 0, x0);
		Traits::store16_raw(dst + 16, x1);
		Traits::store16_raw(dst + 32, x2);
		Traits::store16_raw(dst + 48, x3);
		Traits::store16_raw(dst + 64, x4);
		Traits::store16_raw(dst + 80, x5);
		Traits::store16_raw(dst + 96, x6);
		Traits::store16_raw(dst + 112, x7);
		Traits::store16_raw(dst + 128, x8);
		Traits::store16_raw(dst + 144, x9);
		Traits::store16_raw(dst + 160, x10);
		Traits::store16_raw(dst + 176, x11);
		Traits::store16_raw(dst + 192, x12);
		Traits::store16_raw(dst + 208, x13);
		Traits::store16_raw(dst + 224, x14);
		Traits::store16_raw(dst + 240, x15);

		dst += 256;
	}
}

void transpose_line_32x32_epi16(uint16_t *dst, const uint16_t * const *src, unsigned left, unsigned right)
{
	for (unsigned j = left; j < right; j += 32) {
		__m512i x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
		__m512i x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31;

		x0 = _mm512_load_si512((const __m512i *)(src[0] + j));
		x1 = _mm512_load_si512((const __m512i *)(src[1] + j));
		x2 = _mm512_load_si512((const __m512i *)(src[2] + j));
		x3 = _mm512_load_si512((const __m512i *)(src[3] + j));
		x4 = _mm512_load_si512((const __m512i *)(src[4] + j));
		x5 = _mm512_load_si512((const __m512i *)(src[5] + j));
		x6 = _mm512_load_si512((const __m512i *)(src[6] + j));
		x7 = _mm512_load_si512((const __m512i *)(src[7] + j));
		x8 = _mm512_load_si512((const __m512i *)(src[8] + j));
		x9 = _mm512_load_si512((const __m512i *)(src[9] + j));
		x10 = _mm512_load_si512((const __m512i *)(src[10] + j));
		x11 = _mm512_load_si512((const __m512i *)(src[11] + j));
		x12 = _mm512_load_si512((const __m512i *)(src[12] + j));
		x13 = _mm512_load_si512((const __m512i *)(src[13] + j));
		x14 = _mm512_load_si512((const __m512i *)(src[14] + j));
		x15 = _mm512_load_si512((const __m512i *)(src[15] + j));
		x16 = _mm512_load_si512((const __m512i *)(src[16] + j));
		x17 = _mm512_load_si512((const __m512i *)(src[17] + j));
		x18 = _mm512_load_si512((const __m512i *)(src[18] + j));
		x19 = _mm512_load_si512((const __m512i *)(src[19] + j));
		x20 = _mm512_load_si512((const __m512i *)(src[20] + j));
		x21 = _mm512_load_si512((const __m512i *)(src[21] + j));
		x22 = _mm512_load_si512((const __m512i *)(src[22] + j));
		x23 = _mm512_load_si512((const __m512i *)(src[23] + j));
		x24 = _mm512_load_si512((const __m512i *)(src[24] + j));
		x25 = _mm512_load_si512((const __m512i *)(src[25] + j));
		x26 = _mm512_load_si512((const __m512i *)(src[26] + j));
		x27 = _mm512_load_si512((const __m512i *)(src[27] + j));
		x28 = _mm512_load_si512((const __m512i *)(src[28] + j));
		x29 = _mm512_load_si512((const __m512i *)(src[29] + j));
		x30 = _mm512_load_si512((const __m512i *)(src[30] + j));
		x31 = _mm512_load_si512((const __m512i *)(src[31] + j));

		mm512_transpose32_epi16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15,
		                        x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31);

		_mm512_store_si512((__m512i *)(dst + 0), x0);
		_mm512_store_si512((__m512i *)(dst + 32), x1);
		_mm512_store_si512((__m512i *)(dst + 64), x2);
		_mm512_store_si512((__m512i *)(dst + 96), x3);
		_mm512_store_si512((__m512i *)(dst + 128), x4);
		_mm512_store_si512((__m512i *)(dst + 160), x5);
		_mm512_store_si512((__m512i *)(dst + 192), x6);
		_mm512_store_si512((__m512i *)(dst + 224), x7);
		_mm512_store_si512((__m512i *)(dst + 256), x8);
		_mm512_store_si512((__m512i *)(dst + 288), x9);
		_mm512_store_si512((__m512i *)(dst + 320), x10);
		_mm512_store_si512((__m512i *)(dst + 352), x11);
		_mm512_store_si512((__m512i *)(dst + 384), x12);
		_mm512_store_si512((__m512i *)(dst + 416), x13);
		_mm512_store_si512((__m512i *)(dst + 448), x14);
		_mm512_store_si512((__m512i *)(dst + 480), x15);
		_mm512_store_si512((__m512i *)(dst + 512), x16);
		_mm512_store_si512((__m512i *)(dst + 544), x17);
		_mm512_store_si512((__m512i *)(dst + 576), x18);
		_mm512_store_si512((__m512i *)(dst + 608), x19);
		_mm512_store_si512((__m512i *)(dst + 640), x20);
		_mm512_store_si512((__m512i *)(dst + 672), x21);
		_mm512_store_si512((__m512i *)(dst + 704), x22);
		_mm512_store_si512((__m512i *)(dst + 736), x23);
		_mm512_store_si512((__m512i *)(dst + 768), x24);
		_mm512_store_si512((__m512i *)(dst + 800), x25);
		_mm512_store_si512((__m512i *)(dst + 832), x26);
		_mm512_store_si512((__m512i *)(dst + 864), x27);
		_mm512_store_si512((__m512i *)(dst + 896), x28);
		_mm512_store_si512((__m512i *)(dst + 928), x29);
		_mm512_store_si512((__m512i *)(dst + 960), x30);
		_mm512_store_si512((__m512i *)(dst + 992), x31);

		dst += 1024;
	}
}


template <bool DoLoop, unsigned Tail>
inline FORCE_INLINE __m512i resize_line16_h_u16_avx512_xiter(unsigned j,
                                                             const unsigned *filter_left, const int16_t * RESTRICT filter_data, unsigned filter_stride, unsigned filter_width,
                                                             const uint16_t * RESTRICT src_ptr, unsigned src_base, uint16_t limit)
{
	const __m512i i16_min = _mm512_set1_epi16(INT16_MIN);
	const __m512i lim = _mm512_set1_epi16(limit + INT16_MIN);

	const int16_t *filter_coeffs = filter_data + j * filter_stride;
	const uint16_t *src_p = src_ptr + (filter_left[j] - src_base) * 32;

	__m512i accum_lo = _mm512_setzero_si512();
	__m512i accum_hi = _mm512_setzero_si512();
	__m512i x0, x1, xl, xh, c, coeffs;

	unsigned k_end = DoLoop ? floor_n(filter_width + 1, 8) : 0;

	for (unsigned k = 0; k < k_end; k += 8) {
		coeffs = _mm512_broadcast_i32x4(_mm_load_si128((const __m128i *)(filter_coeffs + k)));

		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_AAAA);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k + 0) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k + 1) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);

		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_BBBB);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k + 2) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k + 3) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);

		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_CCCC);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k + 4) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k + 5) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);

		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_DDDD);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k + 6) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k + 7) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}

	if (Tail >= 2) {
		coeffs = _mm512_broadcast_i32x4(_mm_load_si128((const __m128i *)(filter_coeffs + k_end)));

		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_AAAA);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 0) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 1) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}

	if (Tail >= 4) {
		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_BBBB);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 2) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 3) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}

	if (Tail >= 6) {
		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_CCCC);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 4) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 5) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}

	if (Tail >= 8) {
		c = _mm512_shuffle_epi32(coeffs, _MM_PERM_DDDD);
		x0 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 6) * 32));
		x1 = _mm512_load_si512((const __m256i *)(src_p + (k_end + 7) * 32));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c, xl);
		xh = _mm512_madd_epi16(c, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}

	accum_lo = export_i30_u16(accum_lo, accum_hi, limit);
	accum_lo = _mm512_min_epi16(accum_lo, lim);
	accum_lo = _mm512_sub_epi16(accum_lo, i16_min);
	return accum_lo;
}

template <bool DoLoop, unsigned Tail>
void resize_line16_h_u16_avx512(const unsigned *filter_left, const int16_t * RESTRICT filter_data, unsigned filter_stride, unsigned filter_width,
                                const uint16_t * RESTRICT src_ptr, uint16_t * const *dst_ptr, unsigned src_base, unsigned left, unsigned right, uint16_t limit)
{
	unsigned vec_left = ceil_n(left, 16);
	unsigned vec_right = floor_n(right, 16);

#define XITER resize_line16_h_u16_avx512_xiter<DoLoop, Tail>
#define XARGS filter_left, filter_data, filter_stride, filter_width, src_ptr, src_base, limit
	for (unsigned j = left; j < vec_left; ++j) {
		__m512i x = XITER(j, XARGS);

		mm_scatter_epi16(dst_ptr[0] + j, dst_ptr[1] + j, dst_ptr[2] + j, dst_ptr[3] + j, dst_ptr[4] + j, dst_ptr[5] + j, dst_ptr[6] + j, dst_ptr[7] + j, _mm512_castsi512_si128(x));
		mm_scatter_epi16(dst_ptr[8] + j, dst_ptr[9] + j, dst_ptr[10] + j, dst_ptr[11] + j, dst_ptr[12] + j, dst_ptr[13] + j, dst_ptr[14] + j, dst_ptr[15] + j, _mm512_extracti32x4_epi32(x, 1));
		mm_scatter_epi16(dst_ptr[16] + j, dst_ptr[17] + j, dst_ptr[18] + j, dst_ptr[19] + j, dst_ptr[20] + j, dst_ptr[21] + j, dst_ptr[22] + j, dst_ptr[23] + j, _mm512_extracti32x4_epi32(x, 2));
		mm_scatter_epi16(dst_ptr[24] + j, dst_ptr[25] + j, dst_ptr[26] + j, dst_ptr[27] + j, dst_ptr[28] + j, dst_ptr[29] + j, dst_ptr[30] + j, dst_ptr[31] + j, _mm512_extracti32x4_epi32(x, 3));
	}

	for (unsigned j = vec_left; j < vec_right; j += 32) {
		uint16_t cache alignas(64)[32][32];
		__m512i x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;
		__m512i x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31;

		for (unsigned jj = j; jj < j + 32; ++jj) {
			__m512i x = XITER(jj, XARGS);
			_mm512_store_si512((__m512i *)cache[jj - j], x);
		}

		x0 = _mm512_load_si512((const __m512i *)cache[0]);
		x1 = _mm512_load_si512((const __m512i *)cache[1]);
		x2 = _mm512_load_si512((const __m512i *)cache[2]);
		x3 = _mm512_load_si512((const __m512i *)cache[3]);
		x4 = _mm512_load_si512((const __m512i *)cache[4]);
		x5 = _mm512_load_si512((const __m512i *)cache[5]);
		x6 = _mm512_load_si512((const __m512i *)cache[6]);
		x7 = _mm512_load_si512((const __m512i *)cache[7]);
		x8 = _mm512_load_si512((const __m512i *)cache[8]);
		x9 = _mm512_load_si512((const __m512i *)cache[9]);
		x10 = _mm512_load_si512((const __m512i *)cache[10]);
		x11 = _mm512_load_si512((const __m512i *)cache[11]);
		x12 = _mm512_load_si512((const __m512i *)cache[12]);
		x13 = _mm512_load_si512((const __m512i *)cache[13]);
		x14 = _mm512_load_si512((const __m512i *)cache[14]);
		x15 = _mm512_load_si512((const __m512i *)cache[15]);
		x16 = _mm512_load_si512((const __m512i *)cache[16]);
		x17 = _mm512_load_si512((const __m512i *)cache[17]);
		x18 = _mm512_load_si512((const __m512i *)cache[18]);
		x19 = _mm512_load_si512((const __m512i *)cache[19]);
		x20 = _mm512_load_si512((const __m512i *)cache[20]);
		x21 = _mm512_load_si512((const __m512i *)cache[21]);
		x22 = _mm512_load_si512((const __m512i *)cache[22]);
		x23 = _mm512_load_si512((const __m512i *)cache[23]);
		x24 = _mm512_load_si512((const __m512i *)cache[24]);
		x25 = _mm512_load_si512((const __m512i *)cache[25]);
		x26 = _mm512_load_si512((const __m512i *)cache[26]);
		x27 = _mm512_load_si512((const __m512i *)cache[27]);
		x28 = _mm512_load_si512((const __m512i *)cache[28]);
		x29 = _mm512_load_si512((const __m512i *)cache[29]);
		x30 = _mm512_load_si512((const __m512i *)cache[30]);
		x31 = _mm512_load_si512((const __m512i *)cache[31]);

		mm512_transpose32_epi16(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15,
		                        x16, x17, x18, x19, x20, x21, x22, x23, x24, x25, x26, x27, x28, x29, x30, x31);

		_mm512_store_si512((__m512i *)(dst_ptr[0] + j), x0);
		_mm512_store_si512((__m512i *)(dst_ptr[1] + j), x1);
		_mm512_store_si512((__m512i *)(dst_ptr[2] + j), x2);
		_mm512_store_si512((__m512i *)(dst_ptr[3] + j), x3);
		_mm512_store_si512((__m512i *)(dst_ptr[4] + j), x4);
		_mm512_store_si512((__m512i *)(dst_ptr[5] + j), x5);
		_mm512_store_si512((__m512i *)(dst_ptr[6] + j), x6);
		_mm512_store_si512((__m512i *)(dst_ptr[7] + j), x7);
		_mm512_store_si512((__m512i *)(dst_ptr[8] + j), x8);
		_mm512_store_si512((__m512i *)(dst_ptr[9] + j), x9);
		_mm512_store_si512((__m512i *)(dst_ptr[10] + j), x10);
		_mm512_store_si512((__m512i *)(dst_ptr[11] + j), x11);
		_mm512_store_si512((__m512i *)(dst_ptr[12] + j), x12);
		_mm512_store_si512((__m512i *)(dst_ptr[13] + j), x13);
		_mm512_store_si512((__m512i *)(dst_ptr[14] + j), x14);
		_mm512_store_si512((__m512i *)(dst_ptr[15] + j), x15);
		_mm512_store_si512((__m512i *)(dst_ptr[16] + j), x16);
		_mm512_store_si512((__m512i *)(dst_ptr[17] + j), x17);
		_mm512_store_si512((__m512i *)(dst_ptr[18] + j), x18);
		_mm512_store_si512((__m512i *)(dst_ptr[19] + j), x19);
		_mm512_store_si512((__m512i *)(dst_ptr[20] + j), x20);
		_mm512_store_si512((__m512i *)(dst_ptr[21] + j), x21);
		_mm512_store_si512((__m512i *)(dst_ptr[22] + j), x22);
		_mm512_store_si512((__m512i *)(dst_ptr[23] + j), x23);
		_mm512_store_si512((__m512i *)(dst_ptr[24] + j), x24);
		_mm512_store_si512((__m512i *)(dst_ptr[25] + j), x25);
		_mm512_store_si512((__m512i *)(dst_ptr[26] + j), x26);
		_mm512_store_si512((__m512i *)(dst_ptr[27] + j), x27);
		_mm512_store_si512((__m512i *)(dst_ptr[28] + j), x28);
		_mm512_store_si512((__m512i *)(dst_ptr[29] + j), x29);
		_mm512_store_si512((__m512i *)(dst_ptr[30] + j), x30);
		_mm512_store_si512((__m512i *)(dst_ptr[31] + j), x31);
	}

	for (unsigned j = vec_right; j < right; ++j) {
		__m512i x = XITER(j, XARGS);

		mm_scatter_epi16(dst_ptr[0] + j, dst_ptr[1] + j, dst_ptr[2] + j, dst_ptr[3] + j, dst_ptr[4] + j, dst_ptr[5] + j, dst_ptr[6] + j, dst_ptr[7] + j, _mm512_castsi512_si128(x));
		mm_scatter_epi16(dst_ptr[8] + j, dst_ptr[9] + j, dst_ptr[10] + j, dst_ptr[11] + j, dst_ptr[12] + j, dst_ptr[13] + j, dst_ptr[14] + j, dst_ptr[15] + j, _mm512_extracti32x4_epi32(x, 1));
		mm_scatter_epi16(dst_ptr[16] + j, dst_ptr[17] + j, dst_ptr[18] + j, dst_ptr[19] + j, dst_ptr[20] + j, dst_ptr[21] + j, dst_ptr[22] + j, dst_ptr[23] + j, _mm512_extracti32x4_epi32(x, 2));
		mm_scatter_epi16(dst_ptr[24] + j, dst_ptr[25] + j, dst_ptr[26] + j, dst_ptr[27] + j, dst_ptr[28] + j, dst_ptr[29] + j, dst_ptr[30] + j, dst_ptr[31] + j, _mm512_extracti32x4_epi32(x, 3));
	}
#undef XITER
#undef XARGS
}

const decltype(&resize_line16_h_u16_avx512<false, 0>) resize_line16_h_u16_avx512_jt_small[] = {
	resize_line16_h_u16_avx512<false, 2>,
	resize_line16_h_u16_avx512<false, 2>,
	resize_line16_h_u16_avx512<false, 4>,
	resize_line16_h_u16_avx512<false, 4>,
	resize_line16_h_u16_avx512<false, 6>,
	resize_line16_h_u16_avx512<false, 6>,
	resize_line16_h_u16_avx512<false, 8>,
	resize_line16_h_u16_avx512<false, 8>,
};

const decltype(&resize_line16_h_u16_avx512<false, 0>) resize_line16_h_u16_avx512_jt_large[] = {
	resize_line16_h_u16_avx512<true, 0>,
	resize_line16_h_u16_avx512<true, 2>,
	resize_line16_h_u16_avx512<true, 2>,
	resize_line16_h_u16_avx512<true, 4>,
	resize_line16_h_u16_avx512<true, 4>,
	resize_line16_h_u16_avx512<true, 6>,
	resize_line16_h_u16_avx512<true, 6>,
	resize_line16_h_u16_avx512<true, 0>,
};



template <class Traits, unsigned FWidth, unsigned Tail>
inline FORCE_INLINE __m512 resize_line16_h_fp_avx512_xiter(unsigned j,
                                                           const unsigned *filter_left, const float * RESTRICT filter_data, unsigned filter_stride, unsigned filter_width,
                                                           const typename Traits::pixel_type * RESTRICT src_ptr, unsigned src_base)
{
	typedef typename Traits::pixel_type pixel_type;

	const float *filter_coeffs = filter_data + j * filter_stride;
	const pixel_type *src_p = src_ptr + (filter_left[j] - src_base) * 16;

	__m512 accum0 = _mm512_setzero_ps();
	__m512 accum1 = _mm512_setzero_ps();
	__m512 x, c, coeffs;

	unsigned k_end = FWidth ? FWidth - Tail : floor_n(filter_width, 4);

	for (unsigned k = 0; k < k_end; k += 4) {
		coeffs = _mm512_broadcast_f32x4(_mm_load_ps(filter_coeffs + k));

		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_AAAA);
		x = Traits::load16(src_p + (k + 0) * 16);
		accum0 = _mm512_fmadd_ps(c, x, accum0);

		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_BBBB);
		x = Traits::load16(src_p + (k + 1) * 16);
		accum1 = _mm512_fmadd_ps(c, x, accum1);

		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_CCCC);
		x = Traits::load16(src_p + (k + 2) * 16);
		accum0 = _mm512_fmadd_ps(c, x, accum0);

		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_DDDD);
		x = Traits::load16(src_p + (k + 3) * 16);
		accum1 = _mm512_fmadd_ps(c, x, accum1);
	}

	if (Tail >= 1) {
		coeffs = _mm512_broadcast_f32x4(_mm_load_ps(filter_coeffs + k_end));

		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_AAAA);
		x = Traits::load16(src_p + (k_end + 0) * 16);
		accum0 = _mm512_fmadd_ps(c, x, accum0);
	}
	if (Tail >= 2) {
		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_BBBB);
		x = Traits::load16(src_p + (k_end + 1) * 16);
		accum1 = _mm512_fmadd_ps(c, x, accum1);
	}
	if (Tail >= 3) {
		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_CCCC);
		x = Traits::load16(src_p + (k_end + 2) * 16);
		accum0 = _mm512_fmadd_ps(c, x, accum0);
	}
	if (Tail >= 4) {
		c = _mm512_shuffle_ps(coeffs, coeffs, _MM_PERM_DDDD);
		x = Traits::load16(src_p + (k_end + 3) * 16);
		accum1 = _mm512_fmadd_ps(c, x, accum1);
	}

	if (!FWidth || FWidth >= 2)
		accum0 = _mm512_add_ps(accum0, accum1);

	return accum0;
}

template <class Traits, unsigned FWidth, unsigned Tail>
void resize_line16_h_fp_avx512(const unsigned *filter_left, const float * RESTRICT filter_data, unsigned filter_stride, unsigned filter_width,
                               const typename Traits::pixel_type *src_ptr, typename Traits::pixel_type * const *dst_ptr, unsigned src_base, unsigned left, unsigned right)
{
	unsigned vec_left = ceil_n(left, 16);
	unsigned vec_right = floor_n(right, 16);

#define XITER resize_line16_h_fp_avx512_xiter<Traits, FWidth, Tail>
#define XARGS filter_left, filter_data, filter_stride, filter_width, src_ptr, src_base
	for (unsigned j = left; j < vec_left; ++j) {
		__m512 x = XITER(j, XARGS);
		Traits::scatter16(dst_ptr[0] + j, dst_ptr[1] + j, dst_ptr[2] + j, dst_ptr[3] + j, dst_ptr[4] + j, dst_ptr[5] + j, dst_ptr[6] + j, dst_ptr[7] + j,
		                  dst_ptr[8] + j, dst_ptr[9] + j, dst_ptr[10] + j, dst_ptr[11] + j, dst_ptr[12] + j, dst_ptr[13] + j, dst_ptr[14] + j, dst_ptr[15] + j, x);
	}

	for (unsigned j = vec_left; j < vec_right; j += 16) {
		float cache alignas(64)[16][16];
		__m512 x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15;

		for (unsigned jj = j; jj < j + 16; ++jj) {
			__m512 x = XITER(jj, XARGS);
			_mm512_store_ps(cache[jj - j], x);
		}
		
		x0 = _mm512_load_ps(cache[0]);
		x1 = _mm512_load_ps(cache[1]);
		x2 = _mm512_load_ps(cache[2]);
		x3 = _mm512_load_ps(cache[3]);
		x4 = _mm512_load_ps(cache[4]);
		x5 = _mm512_load_ps(cache[5]);
		x6 = _mm512_load_ps(cache[6]);
		x7 = _mm512_load_ps(cache[7]);
		x8 = _mm512_load_ps(cache[8]);
		x9 = _mm512_load_ps(cache[9]);
		x10 = _mm512_load_ps(cache[10]);
		x11 = _mm512_load_ps(cache[11]);
		x12 = _mm512_load_ps(cache[12]);
		x13 = _mm512_load_ps(cache[13]);
		x14 = _mm512_load_ps(cache[14]);
		x15 = _mm512_load_ps(cache[15]);

		mm512_transpose16_ps(x0, x1, x2, x3, x4, x5, x6, x7, x8, x9, x10, x11, x12, x13, x14, x15);

		Traits::store16(dst_ptr[0] + j, x0);
		Traits::store16(dst_ptr[1] + j, x1);
		Traits::store16(dst_ptr[2] + j, x2);
		Traits::store16(dst_ptr[3] + j, x3);
		Traits::store16(dst_ptr[4] + j, x4);
		Traits::store16(dst_ptr[5] + j, x5);
		Traits::store16(dst_ptr[6] + j, x6);
		Traits::store16(dst_ptr[7] + j, x7);
		Traits::store16(dst_ptr[8] + j, x8);
		Traits::store16(dst_ptr[9] + j, x9);
		Traits::store16(dst_ptr[10] + j, x10);
		Traits::store16(dst_ptr[11] + j, x11);
		Traits::store16(dst_ptr[12] + j, x12);
		Traits::store16(dst_ptr[13] + j, x13);
		Traits::store16(dst_ptr[14] + j, x14);
		Traits::store16(dst_ptr[15] + j, x15);
	}

	for (unsigned j = vec_right; j < right; ++j) {
		__m512 x = XITER(j, XARGS);
		Traits::scatter16(dst_ptr[0] + j, dst_ptr[1] + j, dst_ptr[2] + j, dst_ptr[3] + j, dst_ptr[4] + j, dst_ptr[5] + j, dst_ptr[6] + j, dst_ptr[7] + j,
		                  dst_ptr[8] + j, dst_ptr[9] + j, dst_ptr[10] + j, dst_ptr[11] + j, dst_ptr[12] + j, dst_ptr[13] + j, dst_ptr[14] + j, dst_ptr[15] + j, x);
	}
#undef XITER
#undef XARGS
}

template <class Traits>
struct resize_line16_h_fp_avx512_jt {
	typedef decltype(&resize_line16_h_fp_avx512<Traits, 0, 0>) func_type;

	static const func_type small[8];
	static const func_type large[4];
};

template <class Traits>
const typename resize_line16_h_fp_avx512_jt<Traits>::func_type resize_line16_h_fp_avx512_jt<Traits>::small[8] = {
	resize_line16_h_fp_avx512<Traits, 1, 1>,
	resize_line16_h_fp_avx512<Traits, 2, 2>,
	resize_line16_h_fp_avx512<Traits, 3, 3>,
	resize_line16_h_fp_avx512<Traits, 4, 4>,
	resize_line16_h_fp_avx512<Traits, 5, 1>,
	resize_line16_h_fp_avx512<Traits, 6, 2>,
	resize_line16_h_fp_avx512<Traits, 7, 3>,
	resize_line16_h_fp_avx512<Traits, 8, 4>
};

template <class Traits>
const typename resize_line16_h_fp_avx512_jt<Traits>::func_type resize_line16_h_fp_avx512_jt<Traits>::large[4] = {
	resize_line16_h_fp_avx512<Traits, 0, 0>,
	resize_line16_h_fp_avx512<Traits, 0, 1>,
	resize_line16_h_fp_avx512<Traits, 0, 2>,
	resize_line16_h_fp_avx512<Traits, 0, 3>
};


template <unsigned N, bool ReadAccum, bool WriteToAccum>
inline FORCE_INLINE __m512i resize_line_v_u16_avx512_xiter(unsigned j, unsigned accum_base,
                                                           const uint16_t * RESTRICT src_p0, const uint16_t * RESTRICT src_p1, const uint16_t * RESTRICT src_p2, const uint16_t * RESTRICT src_p3,
                                                           const uint16_t * RESTRICT src_p4, const uint16_t * RESTRICT src_p5, const uint16_t * RESTRICT src_p6, const uint16_t * RESTRICT src_p7,
                                                           const uint32_t *accum_p, const __m512i &c01, const __m512i &c23, const __m512i &c45, const __m512i &c67, uint16_t limit)
{
	const __m512i i16_min = _mm512_set1_epi16(INT16_MIN);
	const __m512i lim = _mm512_set1_epi16(limit + INT16_MIN);

	__m512i accum_lo = _mm512_setzero_si512();
	__m512i accum_hi = _mm512_setzero_si512();
	__m512i x0, x1, xl, xh;

	if (N >= 0) {
		x0 = _mm512_load_si512((const __m512i *)(src_p0 + j));
		x1 = _mm512_load_si512((const __m512i *)(src_p1 + j));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c01, xl);
		xh = _mm512_madd_epi16(c01, xh);

		if (ReadAccum) {
			accum_lo = _mm512_add_epi32(_mm512_load_si512((const __m512i *)(accum_p + j - accum_base + 0)), xl);
			accum_hi = _mm512_add_epi32(_mm512_load_si512((const __m512i *)(accum_p + j - accum_base + 16)), xh);
		} else {
			accum_lo = xl;
			accum_hi = xh;
		}
	}
	if (N >= 2) {
		x0 = _mm512_load_si512((const __m512i *)(src_p2 + j));
		x1 = _mm512_load_si512((const __m512i *)(src_p3 + j));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c23, xl);
		xh = _mm512_madd_epi16(c23, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}
	if (N >= 4) {
		x0 = _mm512_load_si512((const __m512i *)(src_p4 + j));
		x1 = _mm512_load_si512((const __m512i *)(src_p5 + j));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c45, xl);
		xh = _mm512_madd_epi16(c45, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}
	if (N >= 6) {
		x0 = _mm512_load_si512((const __m512i *)(src_p6 + j));
		x1 = _mm512_load_si512((const __m512i *)(src_p7 + j));
		x0 = _mm512_add_epi16(x0, i16_min);
		x1 = _mm512_add_epi16(x1, i16_min);

		xl = _mm512_unpacklo_epi16(x0, x1);
		xh = _mm512_unpackhi_epi16(x0, x1);
		xl = _mm512_madd_epi16(c67, xl);
		xh = _mm512_madd_epi16(c67, xh);

		accum_lo = _mm512_add_epi32(accum_lo, xl);
		accum_hi = _mm512_add_epi32(accum_hi, xh);
	}

	if (WriteToAccum) {
		_mm512_store_si512((__m512i *)(accum_p + j - accum_base + 0), accum_lo);
		_mm512_store_si512((__m512i *)(accum_p + j - accum_base + 16), accum_hi);
		return _mm512_setzero_si512();
	} else {
		accum_lo = export_i30_u16(accum_lo, accum_hi, limit);
		accum_lo = _mm512_min_epi16(accum_lo, lim);
		accum_lo = _mm512_sub_epi16(accum_lo, i16_min);

		return accum_lo;
	}
}

template <unsigned N, bool ReadAccum, bool WriteToAccum>
void resize_line_v_u16_avx512(const int16_t *filter_data, const uint16_t * const *src_lines, uint16_t *dst, uint32_t *accum, unsigned left, unsigned right, uint16_t limit)
{
	const uint16_t * RESTRICT src_p0 = src_lines[0];
	const uint16_t * RESTRICT src_p1 = src_lines[1];
	const uint16_t * RESTRICT src_p2 = src_lines[2];
	const uint16_t * RESTRICT src_p3 = src_lines[3];
	const uint16_t * RESTRICT src_p4 = src_lines[4];
	const uint16_t * RESTRICT src_p5 = src_lines[5];
	const uint16_t * RESTRICT src_p6 = src_lines[6];
	const uint16_t * RESTRICT src_p7 = src_lines[7];
	uint16_t * RESTRICT dst_p = dst;
	uint32_t * RESTRICT accum_p = accum;

	unsigned vec_left = ceil_n(left, 32);
	unsigned vec_right = floor_n(right, 32);
	unsigned accum_base = floor_n(left, 32);

	const __m512i c01 = _mm512_unpacklo_epi16(_mm512_set1_epi16(filter_data[0]), _mm512_set1_epi16(filter_data[1]));
	const __m512i c23 = _mm512_unpacklo_epi16(_mm512_set1_epi16(filter_data[2]), _mm512_set1_epi16(filter_data[3]));
	const __m512i c45 = _mm512_unpacklo_epi16(_mm512_set1_epi16(filter_data[4]), _mm512_set1_epi16(filter_data[5]));
	const __m512i c67 = _mm512_unpacklo_epi16(_mm512_set1_epi16(filter_data[6]), _mm512_set1_epi16(filter_data[7]));

	__m512i out;

#define XITER resize_line_v_u16_avx512_xiter<N, ReadAccum, WriteToAccum>
#define XARGS accum_base, src_p0, src_p1, src_p2, src_p3, src_p4, src_p5, src_p6, src_p7, accum_p, c01, c23, c45, c67, limit
	if (left != vec_left) {
		out = XITER(vec_left - 32, XARGS);

		if (!WriteToAccum)
			_mm512_mask_storeu_epi16(dst + vec_left - 32, 0xFFFFFFFFU << (32 - (vec_left - left)), out);
	}

	for (unsigned j = vec_left; j < vec_right; j += 32) {
		out = XITER(j, XARGS);

		if (!WriteToAccum)
			_mm512_store_si512((__m512i *)(dst_p + j), out);
	}

	if (right != vec_right) {
		out = XITER(vec_right, XARGS);

		if (!WriteToAccum)
			_mm512_mask_storeu_epi16(dst + vec_right, 0xFFFFFFFFU >> (32 - (right - vec_right)), out);
	}
#undef XITER
#undef XARGS
}

const decltype(&resize_line_v_u16_avx512<0, false, false>) resize_line_v_u16_avx512_jt_a[] = {
	resize_line_v_u16_avx512<0, false, false>,
	resize_line_v_u16_avx512<0, false, false>,
	resize_line_v_u16_avx512<2, false, false>,
	resize_line_v_u16_avx512<2, false, false>,
	resize_line_v_u16_avx512<4, false, false>,
	resize_line_v_u16_avx512<4, false, false>,
	resize_line_v_u16_avx512<6, false, false>,
	resize_line_v_u16_avx512<6, false, false>,
};

const decltype(&resize_line_v_u16_avx512<0, false, false>) resize_line_v_u16_avx512_jt_b[] = {
	resize_line_v_u16_avx512<0, true, false>,
	resize_line_v_u16_avx512<0, true, false>,
	resize_line_v_u16_avx512<2, true, false>,
	resize_line_v_u16_avx512<2, true, false>,
	resize_line_v_u16_avx512<4, true, false>,
	resize_line_v_u16_avx512<4, true, false>,
	resize_line_v_u16_avx512<6, true, false>,
	resize_line_v_u16_avx512<6, true, false>,
};


template <class Traits, unsigned N, bool UpdateAccum, class T = typename Traits::pixel_type>
inline FORCE_INLINE __m512 resize_line_v_fp_avx512_xiter(unsigned j,
                                                         const T * RESTRICT src_p0, const T * RESTRICT src_p1,
                                                         const T * RESTRICT src_p2, const T * RESTRICT src_p3,
                                                         const T * RESTRICT src_p4, const T * RESTRICT src_p5,
                                                         const T * RESTRICT src_p6, const T * RESTRICT src_p7, T * RESTRICT dst_p,
                                                         const __m512 &c0, const __m512 &c1, const __m512 &c2, const __m512 &c3,
                                                         const __m512 &c4, const __m512 &c5, const __m512 &c6, const __m512 &c7)
{
	typedef typename Traits::pixel_type pixel_type;
	static_assert(std::is_same<pixel_type, T>::value, "must not specify T");

	__m512 accum0 = _mm512_setzero_ps();
	__m512 accum1 = _mm512_setzero_ps();
	__m512 x;

	if (N >= 0) {
		x = Traits::load16(src_p0 + j);
		accum0 = UpdateAccum ? _mm512_fmadd_ps(c0, x, Traits::load16(dst_p + j)) : _mm512_mul_ps(c0, x);
	}
	if (N >= 1) {
		x = Traits::load16(src_p1 + j);
		accum1 = _mm512_mul_ps(c1, x);
	}
	if (N >= 2) {
		x = Traits::load16(src_p2 + j);
		accum0 = _mm512_fmadd_ps(c2, x, accum0);
	}
	if (N >= 3) {
		x = Traits::load16(src_p3 + j);
		accum1 = _mm512_fmadd_ps(c3, x, accum1);
	}
	if (N >= 4) {
		x = Traits::load16(src_p4 + j);
		accum0 = _mm512_fmadd_ps(c4, x, accum0);
	}
	if (N >= 5) {
		x = Traits::load16(src_p5 + j);
		accum1 = _mm512_fmadd_ps(c5, x, accum1);
	}
	if (N >= 6) {
		x = Traits::load16(src_p6 + j);
		accum0 = _mm512_fmadd_ps(c6, x, accum0);
	}
	if (N >= 7) {
		x = Traits::load16(src_p7 + j);
		accum1 = _mm512_fmadd_ps(c7, x, accum1);
	}

	accum0 = (N >= 1) ? _mm512_add_ps(accum0, accum1) : accum0;
	return accum0;
}

template <class Traits, unsigned N, bool UpdateAccum>
void resize_line_v_fp_avx512(const float *filter_data, const typename Traits::pixel_type * const *src_lines, typename Traits::pixel_type *dst, unsigned left, unsigned right)
{
	typedef typename Traits::pixel_type pixel_type;

	const pixel_type * RESTRICT src_p0 = src_lines[0];
	const pixel_type * RESTRICT src_p1 = src_lines[1];
	const pixel_type * RESTRICT src_p2 = src_lines[2];
	const pixel_type * RESTRICT src_p3 = src_lines[3];
	const pixel_type * RESTRICT src_p4 = src_lines[4];
	const pixel_type * RESTRICT src_p5 = src_lines[5];
	const pixel_type * RESTRICT src_p6 = src_lines[6];
	const pixel_type * RESTRICT src_p7 = src_lines[7];
	pixel_type * RESTRICT dst_p = dst;

	unsigned vec_left = ceil_n(left, 16);
	unsigned vec_right = floor_n(right, 16);

	const __m512 c0 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 0));
	const __m512 c1 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 1));
	const __m512 c2 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 2));
	const __m512 c3 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 3));
	const __m512 c4 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 4));
	const __m512 c5 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 5));
	const __m512 c6 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 6));
	const __m512 c7 = _mm512_broadcastss_ps(_mm_load_ss(filter_data + 7));

	__m512 accum;

#define XITER resize_line_v_fp_avx512_xiter<Traits, N, UpdateAccum>
#define XARGS src_p0, src_p1, src_p2, src_p3, src_p4, src_p5, src_p6, src_p7, dst_p, c0, c1, c2, c3, c4, c5, c6, c7
	if (left != vec_left) {
		accum = XITER(vec_left - 16, XARGS);
		Traits::mask_store16(dst + vec_left - 16, 0xFFFFU << (16 - (vec_left - left)), accum);
	}
	for (unsigned j = vec_left; j < vec_right; j += 16) {
		accum = XITER(j, XARGS);
		Traits::mask_store16(dst + j, 0xFFFFU, accum);
	}
	if (right != vec_right) {
		accum = XITER(vec_right, XARGS);
		Traits::mask_store16(dst + vec_right, 0xFFFFU >> (16 - (right - vec_right)), accum);
	}
#undef XITER
#undef XARGS
}

template <class Traits>
struct resize_line_v_fp_avx512_jt {
	typedef decltype(&resize_line_v_fp_avx512<Traits, 0, false>) func_type;

	static const func_type table_a[8];
	static const func_type table_b[8];
};

template <class Traits>
const typename resize_line_v_fp_avx512_jt<Traits>::func_type resize_line_v_fp_avx512_jt<Traits>::table_a[8] = {
	resize_line_v_fp_avx512<Traits, 0, false>,
	resize_line_v_fp_avx512<Traits, 1, false>,
	resize_line_v_fp_avx512<Traits, 2, false>,
	resize_line_v_fp_avx512<Traits, 3, false>,
	resize_line_v_fp_avx512<Traits, 4, false>,
	resize_line_v_fp_avx512<Traits, 5, false>,
	resize_line_v_fp_avx512<Traits, 6, false>,
	resize_line_v_fp_avx512<Traits, 7, false>,
};

template <class Traits>
const typename resize_line_v_fp_avx512_jt<Traits>::func_type resize_line_v_fp_avx512_jt<Traits>::table_b[8] = {
	resize_line_v_fp_avx512<Traits, 0, true>,
	resize_line_v_fp_avx512<Traits, 1, true>,
	resize_line_v_fp_avx512<Traits, 2, true>,
	resize_line_v_fp_avx512<Traits, 3, true>,
	resize_line_v_fp_avx512<Traits, 4, true>,
	resize_line_v_fp_avx512<Traits, 5, true>,
	resize_line_v_fp_avx512<Traits, 6, true>,
	resize_line_v_fp_avx512<Traits, 7, true>,
};


inline FORCE_INLINE void calculate_line_address(void *dst, const void *src, ptrdiff_t stride, unsigned mask, unsigned i, unsigned height)
{
	__m512i idx = _mm512_set1_epi64(i);
	__m512i m = _mm512_set1_epi64(mask);
	__m512i p = _mm512_set1_epi64(reinterpret_cast<intptr_t>(src));

	idx = _mm512_add_epi64(idx, _mm512_set_epi64(7, 6, 5, 4, 3, 2, 1, 0));
	idx = _mm512_min_epi64(idx, _mm512_set1_epi64(height - 1));
	idx = _mm512_and_epi64(idx, m);
	idx = _mm512_mullo_epi64(idx, _mm512_set1_epi64(stride));

	p = _mm512_add_epi64(p, idx);
#if defined(_M_X64) || defined(__X86_64__)
	_mm512_store_si512((__m512i *)dst, p);
#else
	_mm256_store_si256((__m256i *)dst, _mm512_cvtepi64_epi32(p));
#endif
}


class ResizeImplH_U16_AVX512 final : public ResizeImplH {
	decltype(&resize_line16_h_u16_avx512<false, 0>) m_func;
	uint16_t m_pixel_max;
public:
	ResizeImplH_U16_AVX512(const FilterContext &filter, unsigned height, unsigned depth) :
		ResizeImplH(filter, image_attributes{ filter.filter_rows, height, PixelType::WORD }),
		// m_func{},
		m_pixel_max{ static_cast<uint16_t>((1UL << depth) - 1) }
	{
		if (filter.filter_width > 8)
			m_func = resize_line16_h_u16_avx512_jt_large[filter.filter_width % 8];
		else
			m_func = resize_line16_h_u16_avx512_jt_small[filter.filter_width - 1];
	}

	unsigned get_simultaneous_lines() const override { return 32; }

	size_t get_tmp_size(unsigned left, unsigned right) const override
	{
		auto range = get_required_col_range(left, right);

		try {
			checked_size_t size = (static_cast<checked_size_t>(range.second) - floor_n(range.first, 32) + 32) * sizeof(uint16_t) * 32;
			return size.get();
		} catch (const std::overflow_error &) {
			error::throw_<error::OutOfMemory>();
		}
	}

	void process(void *, const graph::ImageBuffer<const void> *src, const graph::ImageBuffer<void> *dst, void *tmp, unsigned i, unsigned left, unsigned right) const override
	{
		auto range = get_required_col_range(left, right);

		alignas(64) const uint16_t *src_ptr[32];
		alignas(64) uint16_t *dst_ptr[32];
		uint16_t *transpose_buf = static_cast<uint16_t *>(tmp);
		unsigned height = get_image_attributes().height;

		calculate_line_address(src_ptr + 0, src->data(), src->stride(), src->mask(), i + 0, height);
		calculate_line_address(src_ptr + 8, src->data(), src->stride(), src->mask(), i + std::min(8U, height - i - 1), height);
		calculate_line_address(src_ptr + 16, src->data(), src->stride(), src->mask(), i + std::min(16U, height - i - 1), height);
		calculate_line_address(src_ptr + 24, src->data(), src->stride(), src->mask(), i + std::min(24U, height - i - 1), height);

		transpose_line_32x32_epi16(transpose_buf, src_ptr, floor_n(range.first, 32), ceil_n(range.second, 32));

		calculate_line_address(dst_ptr + 0, dst->data(), dst->stride(), dst->mask(), i + 0, height);
		calculate_line_address(dst_ptr + 8, dst->data(), dst->stride(), dst->mask(), i + std::min(8U, height - i - 1), height);
		calculate_line_address(dst_ptr + 16, dst->data(), dst->stride(), dst->mask(), i + std::min(16U, height - i - 1), height);
		calculate_line_address(dst_ptr + 24, dst->data(), dst->stride(), dst->mask(), i + std::min(24U, height - i - 1), height);

		m_func(m_filter.left.data(), m_filter.data_i16.data(), m_filter.stride_i16, m_filter.filter_width,
		       transpose_buf, dst_ptr, floor_n(range.first, 32), left, right, m_pixel_max);
	}
};

template <class Traits>
class ResizeImplH_FP_AVX512 final : public ResizeImplH {
	typedef typename Traits::pixel_type pixel_type;
	typedef typename resize_line16_h_fp_avx512_jt<Traits>::func_type func_type;

	func_type m_func;
public:
	ResizeImplH_FP_AVX512(const FilterContext &filter, unsigned height) :
		ResizeImplH(filter, image_attributes{ filter.filter_rows, height, Traits::type_constant }),
		m_func{}
	{
		if (filter.filter_width <= 8)
			m_func = resize_line16_h_fp_avx512_jt<Traits>::small[filter.filter_width - 1];
		else
			m_func = resize_line16_h_fp_avx512_jt<Traits>::large[filter.filter_width % 4];
	}

	unsigned get_simultaneous_lines() const override { return 16; }

	size_t get_tmp_size(unsigned left, unsigned right) const override
	{
		auto range = get_required_col_range(left, right);

		try {
			checked_size_t size = (static_cast<checked_size_t>(range.second) - floor_n(range.first, 16) + 16) * sizeof(pixel_type) * 16;
			return size.get();
		} catch (const std::overflow_error &) {
			error::throw_<error::OutOfMemory>();
		}
	}

	void process(void *, const graph::ImageBuffer<const void> *src, const graph::ImageBuffer<void> *dst, void *tmp, unsigned i, unsigned left, unsigned right) const override
	{
		auto range = get_required_col_range(left, right);

		alignas(64) const pixel_type *src_ptr[16];
		alignas(64) pixel_type *dst_ptr[16];
		pixel_type *transpose_buf = static_cast<pixel_type *>(tmp);
		unsigned height = get_image_attributes().height;

		calculate_line_address(src_ptr + 0, src->data(), src->stride(), src->mask(), i + 0, height);
		calculate_line_address(src_ptr + 8, src->data(), src->stride(), src->mask(), i + std::min(8U, height - i - 1), height);

		transpose_line_16x16<Traits>(transpose_buf, src_ptr, floor_n(range.first, 16), ceil_n(range.second, 16));

		calculate_line_address(dst_ptr + 0, dst->data(), dst->stride(), dst->mask(), i + 0, height);
		calculate_line_address(dst_ptr + 8, dst->data(), dst->stride(), dst->mask(), i + std::min(8U, height - i - 1), height);

		m_func(m_filter.left.data(), m_filter.data.data(), m_filter.stride, m_filter.filter_width,
		       transpose_buf, dst_ptr, floor_n(range.first, 16), left, right);
	}
};

class ResizeImplV_U16_AVX512 final : public ResizeImplV {
	uint16_t m_pixel_max;
public:
	ResizeImplV_U16_AVX512(const FilterContext &filter, unsigned width, unsigned depth) :
		ResizeImplV(filter, image_attributes{ width, filter.filter_rows, PixelType::WORD }),
		m_pixel_max{ static_cast<uint16_t>((1UL << depth) - 1) }
	{}

	size_t get_tmp_size(unsigned left, unsigned right) const override
	{
		checked_size_t size = 0;

		try {
			if (m_filter.filter_width > 8)
				size += (ceil_n(checked_size_t{ right }, 32) - floor_n(left, 32)) * sizeof(uint32_t);
		} catch (const std::overflow_error &) {
			error::throw_<error::OutOfMemory>();
		}

		return size.get();
	}

	void process(void *, const graph::ImageBuffer<const void> *src, const graph::ImageBuffer<void> *dst, void *tmp, unsigned i, unsigned left, unsigned right) const override
	{
		const auto &dst_buf = graph::static_buffer_cast<uint16_t>(*dst);

		const int16_t *filter_data = m_filter.data_i16.data() + i * m_filter.stride_i16;
		unsigned filter_width = m_filter.filter_width;
		unsigned src_height = m_filter.input_width;

		alignas(64) const uint16_t *src_lines[8];
		uint16_t *dst_line = dst_buf[i];
		uint32_t *accum_buf = static_cast<uint32_t *>(tmp);

		unsigned top = m_filter.left[i];

		if (filter_width <= 8) {
			calculate_line_address(src_lines, src->data(), src->stride(), src->mask(), top + 0, src_height);
			resize_line_v_u16_avx512_jt_a[filter_width - 1](filter_data, src_lines, dst_line, accum_buf, left, right, m_pixel_max);
		} else {
			unsigned k_end = ceil_n(filter_width, 8) - 8;

			calculate_line_address(src_lines, src->data(), src->stride(), src->mask(), top + 0, src_height);
			resize_line_v_u16_avx512<6, false, true>(filter_data + 0, src_lines, dst_line, accum_buf, left, right, m_pixel_max);

			for (unsigned k = 8; k < k_end; k += 8) {
				calculate_line_address(src_lines, src->data(), src->stride(), src->mask(), top + k, src_height);
				resize_line_v_u16_avx512<6, true, true>(filter_data + k, src_lines, dst_line, accum_buf, left, right, m_pixel_max);
			}

			calculate_line_address(src_lines, src->data(), src->stride(), src->mask(), top + k_end, src_height);
			resize_line_v_u16_avx512_jt_b[filter_width - k_end - 1](filter_data + k_end, src_lines, dst_line, accum_buf, left, right, m_pixel_max);
		}
	}
};

template <class Traits>
class ResizeImplV_FP_AVX512 final : public ResizeImplV {
	typedef typename Traits::pixel_type pixel_type;
public:
	ResizeImplV_FP_AVX512(const FilterContext &filter, unsigned width) :
		ResizeImplV(filter, image_attributes{ width, filter.filter_rows, Traits::type_constant })
	{}

	void process(void *, const graph::ImageBuffer<const void> *src, const graph::ImageBuffer<void> *dst, void *, unsigned i, unsigned left, unsigned right) const override
	{
		const auto &dst_buf = graph::static_buffer_cast<pixel_type>(*dst);

		const float *filter_data = m_filter.data.data() + i * m_filter.stride;
		unsigned filter_width = m_filter.filter_width;
		unsigned src_height = m_filter.input_width;

		alignas(64) const pixel_type *src_lines[8];
		pixel_type *dst_line = dst_buf[i];

		{
			unsigned taps_remain = std::min(filter_width - 0, 8U);
			unsigned top = m_filter.left[i] + 0;

			calculate_line_address(src_lines, src->data(), src->stride(), src->mask(), top, src_height);
			resize_line_v_fp_avx512_jt<Traits>::table_a[taps_remain - 1](filter_data + 0, src_lines, dst_line, left, right);
		}

		for (unsigned k = 8; k < filter_width; k += 8) {
			unsigned taps_remain = std::min(filter_width - k, 8U);
			unsigned top = m_filter.left[i] + k;

			calculate_line_address(src_lines, src->data(), src->stride(), src->mask(), top, src_height);
			resize_line_v_fp_avx512_jt<Traits>::table_b[taps_remain - 1](filter_data + k, src_lines, dst_line, left, right);
		}
	}
};

} // namespace


std::unique_ptr<graph::ImageFilter> create_resize_impl_h_avx512(const FilterContext &context, unsigned height, PixelType type, unsigned depth)
{
	std::unique_ptr<graph::ImageFilter> ret;

	if (type == PixelType::WORD)
		ret = ztd::make_unique<ResizeImplH_U16_AVX512>(context, height, depth);
	else if (type == PixelType::HALF)
		ret = ztd::make_unique<ResizeImplH_FP_AVX512<f16_traits>>(context, height);
	else if (type == PixelType::FLOAT)
		ret = ztd::make_unique<ResizeImplH_FP_AVX512<f32_traits>>(context, height);

	return ret;
}

std::unique_ptr<graph::ImageFilter> create_resize_impl_v_avx512(const FilterContext &context, unsigned width, PixelType type, unsigned depth)
{
	std::unique_ptr<graph::ImageFilter> ret;

	if (type == PixelType::WORD)
		ret = ztd::make_unique<ResizeImplV_U16_AVX512>(context, width, depth);
	else if (type == PixelType::HALF)
		ret = ztd::make_unique<ResizeImplV_FP_AVX512<f16_traits>>(context, width);
	else if (type == PixelType::FLOAT)
		ret = ztd::make_unique<ResizeImplV_FP_AVX512<f32_traits>>(context, width);

	return ret;
}

} // namespace resize
} // namespace zimg

#endif // ZIMG_X86_AVX512