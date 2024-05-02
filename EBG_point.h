#pragma once

#include <ostream>
#include <algorithm>

#include "EBG_point_op_macros.h"

namespace ebg
{
	template <typename T>
	struct point
	{
		T x, y;

		constexpr point() : x(0), y(0) {}
		constexpr point(T xIn, T yIn) : x(xIn), y(yIn) {}

		template <typename U>
		constexpr point(point<U> p) : x(static_cast<T>(p.x)), y(static_cast<T>(p.y)) {}
	};

	template <typename T>
	using vec2 = point<T>;

	typedef point<int> ipoint, ivec2;
	typedef point<unsigned> upoint, uvec2;
	typedef point<float> fpoint, fvec2;

	template <typename T>
	struct vec3
	{
		T x, y, z;

		constexpr vec3() : x(0), y(0), z(0) {}
		constexpr vec3(T vIn) : x(vIn), y(vIn), z(vIn) {}
		constexpr vec3(T xIn, T yIn, T zIn) : x(xIn), y(yIn), z(zIn) {}
		constexpr vec3(vec2<T> v2, T zIn) : x(v2.x), y(v2.y), z(zIn) {}

		template <typename U>
		constexpr vec3(vec3<U> v3) :
			x(static_cast<T>(v3.x)),
			y(static_cast<T>(v3.y)),
			z(static_cast<T>(v3.z)) {}
	};

	typedef vec3<int> ivec3;
	typedef vec3<unsigned> uvec3;
	typedef vec3<float> fvec3;

	inline constexpr float magnitude_square(fvec2 v)
	{
		return v.x * v.x + v.y * v.y;
	}
	inline constexpr float magnitude_square(fvec3 v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}
	inline float magnitude(fvec2 v)
	{
		return sqrtf(magnitude_square(v));
	}
	inline float magnitude(fvec3 v)
	{
		return sqrtf(magnitude_square(v));
	}

	inline fvec2 round(fvec2 v)
	{
		return fvec2(roundf(v.x), roundf(v.y));
	}
	inline fvec3 round(fvec3 v)
	{
		return fvec3(roundf(v.x), roundf(v.y), roundf(v.z));
	}

	template <typename T>
	inline constexpr T dot(vec2<T> a, vec2<T> b)
	{
		return a.x * b.x + a.y * b.y;
	}
	inline constexpr float dot(fvec3 a, fvec3 b)
	{
		return a.x * b.x + a.y * b.y + a.z * b.z;
	}

	constexpr fvec3 cross(fvec3 a, fvec3 b)
	{
		return fvec3(
			a.y * b.z - a.z * b.y,
			a.z * b.x - a.x * b.z,
			a.x * b.y - a.y * b.x
		);
	}

	inline ipoint clamp(ipoint p, ipoint lo, ipoint hi)
	{
		return ipoint(std::clamp(p.x, lo.x, hi.x), std::clamp(p.y, lo.y, hi.y));
	}
	inline ipoint clamp(ipoint p, ipoint hi)
	{
		return ipoint(std::clamp(p.x, 0, hi.x), std::clamp(p.y, 0, hi.y));
	}

	inline void swap_if(ipoint& lo, ipoint& hi)
	{
		if (lo.x > hi.x)
			std::swap(lo.x, hi.x);
		if (lo.y > hi.y)
			std::swap(lo.y, hi.y);
	}

	void operator |(ipoint& lo, ipoint& hi) { swap_if(lo, hi); }

	bool operator ^=(ipoint a, ipoint b)
	{
		return (a.x == b.x) || (a.y == b.y);
	}

	template <typename T>
	std::ostream& operator <<(std::ostream& os, point<T> p)
	{
		os << '(' << p.x << ", " << p.y << ')';
		return os;
	}

	template <typename T>
	std::ostream& operator <<(std::ostream& os, vec3<T> v)
	{
		os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
		return os;
	}

	template <typename T>
	constexpr point<T> operator -(point<T> p)
	{
		return point<T>(-p.x, -p.y);
	}

	template <typename T>
	constexpr vec3<T> operator -(vec3<T> v)
	{
		return vec3<T>(-v.x, -v.y, -v.z);
	}

	template <typename T>
	constexpr point<T> operator ~(point<T> p)
	{
		return point<T>(p.y, p.x);
	}

	ALL_OPS2
	ALL_OPS3

	inline constexpr bool is_inside(ipoint p, upoint hi)
	{
		return (upoint)p < hi;
	}

	inline fvec3 normalize(fvec3 v)
	{
		return v / magnitude(v);
	}
}

#include "EBG_point_op_undef.h"
