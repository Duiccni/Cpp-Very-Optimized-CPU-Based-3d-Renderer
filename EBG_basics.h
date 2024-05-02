#pragma once

#include "EBG_point.h"
#include "EBG_colors.h"
#include <bit>

#include <cassert>

#define TYPE_MALLOC(T, size) reinterpret_cast<T*>(malloc((size) * sizeof(T)))

namespace ebg
{
	inline unsigned sign_mask(int x) { return x >> 0x1F; }
	inline int get_sign(int x) { return sign_mask(x) | 1; }

	inline int modulo(int x, int a) { return (x % a) + (sign_mask(x ^ a) & a); }

	inline uint8_t slide_uint8(uint8_t x1, uint8_t x2, uint8_t t)
	{
		return x1 + (x2 - x1) * t / UINT8_MAX;
	}

	namespace data
	{
		constexpr unsigned cb_size = 0xFFFFU;
		uint8_t* cb = nullptr;

		inline void init_cb()
		{
			assert(cb == nullptr);
			cb = TYPE_MALLOC(uint8_t, cb_size);
		}

		inline void free_cb()
		{
			assert(cb != nullptr);
			free(cb);
			cb = nullptr;
		}
	}
}
