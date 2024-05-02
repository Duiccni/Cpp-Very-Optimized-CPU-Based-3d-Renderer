#pragma once

namespace ebg
{
	typedef unsigned color_t;

	namespace colors
	{
		constexpr color_t
			alpha = 0xFF000000,
			black = alpha,
			transparent = 0,
			white = 0xFFFFFFFF,
			light_gray = 0xC0C0C0,
			dark_gray = 0x404040,
			red = 0xFFFF0000,
			green = 0xFF00FF00,
			blue = 0xFF0000FF,
			gray = 0x808080;
	}
}