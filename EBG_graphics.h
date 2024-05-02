#pragma once

#include "EBG_basics.h"

#define EPSILON 0.125f

namespace ebg
{
	namespace graphics
	{
		inline constexpr color_t rgba_color(
			uint8_t r, uint8_t g, uint8_t b, uint8_t a = UINT8_MAX)
		{
			return b | (g << 010) | (r << 020) | (a << 030);
		}

		struct surface
		{
			color_t* buffer, * end;
			upoint dim;
			unsigned buffer_size;

			constexpr surface() : buffer(nullptr), end(nullptr), dim(), buffer_size(0) {}
			surface(upoint dimIn, bool alloc = true) : dim(dimIn)
			{
				buffer_size = dim.x * dim.y;
				if (alloc)
				{
					buffer = TYPE_MALLOC(color_t, buffer_size);
					end = buffer + buffer_size;
					return;
				}

				buffer = end = nullptr;
			}
		};

		inline void delete_surface(surface* surf)
		{
			free(surf->buffer);
			surf->buffer = nullptr;
		}
		inline void copy_surface(surface* src, surface* dest)
		{
			memcpy(dest->buffer, src->buffer, src->buffer_size << 2);
		}
		void operator>>(surface& src, surface& dest)
		{
			memcpy(dest.buffer, src.buffer, src.buffer_size << 2);
		}
		inline void fill_grayscale(uint8_t c, surface* surf)
		{
			memset(surf->buffer, c, surf->buffer_size << 2);
		}
		void operator>>(uint8_t c, surface& surf)
		{
			memset(surf.buffer, c, surf.buffer_size << 2);
		}
	}

	namespace data
	{
		graphics::surface cs;

		void init()
		{
			init_cb();
			cs.buffer = reinterpret_cast<color_t*>(cb);
			cs.buffer_size = cb_size >> 2;
			cs.end = cs.buffer + cs.buffer_size;
			cs.dim = upoint(cs.buffer_size, 1);
		}
	}

	namespace graphics
	{
#define GET_PIXEL(p, s) s->buffer[p.x + p.y * s->dim.x]

		inline color_t* get_raw_pixel(upoint p, surface* surf)
		{
			return &GET_PIXEL(p, surf);
		}
		inline color_t get_raw_color(upoint p, surface* surf)
		{
			return GET_PIXEL(p, surf);
		}
		inline color_t* get_pixel(upoint p, surface* surf)
		{
			return is_inside(p, surf->dim) ? &GET_PIXEL(p, surf) : nullptr;
		}
		inline color_t get_color(upoint p, surface* surf)
		{
			return is_inside(p, surf->dim) ? GET_PIXEL(p, surf) : 0;
		}
		inline void set_sure_pixel(upoint p, color_t color, surface* surf)
		{
			GET_PIXEL(p, surf) = color;
		}
		inline void set_pixel(upoint p, color_t color, surface* surf)
		{
			if (is_inside(p, surf->dim))
				GET_PIXEL(p, surf) = color;
		}

#undef GET_PIXEL

		namespace draw
		{
			void straight_line(int d1, int d2, int s, bool slope, color_t color, surface* surf)
			{
				if (s < 0 || d1 == d2) return;

				upoint dim, steps;

				if (slope)
				{
					dim = ~surf->dim;
					steps = ipoint(surf->dim.x, 1);
				}
				else
				{
					dim = surf->dim;
					steps = ipoint(1, surf->dim.x);
				}

				if (static_cast<unsigned>(s) >= dim.y) return;

				d1 = std::clamp(d1, 0, static_cast<int>(dim.x));
				d2 = std::clamp(d2, 0, static_cast<int>(dim.x));

				if (d1 == d2) return;
				if (d1 > d2) std::swap(d1, d2);

				unsigned offset_temp = s * steps.y;

				for (color_t* px = &surf->buffer[d1 * steps.x + offset_temp],
					*end = &surf->buffer[d2 * steps.x + offset_temp]; px < end; px += steps.x)
					*px = color;
			}

			inline void straight_line(ipoint start, ipoint end, color_t color, surface* surf)
			{
				if (start.y == end.y)
					return straight_line(start.x, end.x, start.y, false, color, surf);
				if (start.x == end.x)
					return straight_line(start.y, end.y, start.x, true, color, surf);
			}

			void line(ipoint start, ipoint end, color_t color, surface* surf, bool clip_inside = true)
			{
				ipoint d = end - start;

				if (d.y == 0)
				{
					if (d.x != 0)
						straight_line(start.x, end.x, start.y, false, color, surf);
					return;
				}

				if (d.x == 0)
				{
					if (d.y != 0)
						straight_line(start.y, end.y, start.x, true, color, surf);
					return;
				}

				if (start.x > end.x)
					std::swap(start, end), d = -d;

				ipoint idim = surf->dim;
				bool start_in = is_inside(start, surf->dim), end_in = is_inside(end, surf->dim);

				/*
				FIXED (13/3/24 11.41pm) Not sure
				BUG: (12/4/24 05.18am)
				when slope is bigger than 45 degrees and both points out from surface
				dx == |dx|
				|dy| > dx
				something weird happens:
				code is painting staring point but not painting end point
				but we are clamping both points by window dimension
				when start.x become bigger than dimension.x
				we set start.x to dimension.x
				but thats makes overflow then a single pixel come out from other side of window
				but this is a little problem I cant give even a bit computer power to fix this

				bug condutions:
				|dy| > |dx|
				start.y < 0
				end.x > dim.x

				HOWEVER its does not overflow whole buffer its just overflowing that line of y-axis
				except for only 1 position (in theory, I didnt test it)
				*/

				if (clip_inside)
				{
					/*
					f(x) = ax + c
					f^-1(y) = (y - c) / a
					a = dy/dx
					c = y - ax
					c = start.y - start.x * dy/dx

					(-c/a, 0) = (f^-1(0), 0)

					(f^-1(dim.y), dim.y)
					f^-1(dim.y) = (dim.y - c) / a
					= (dim.y - start.y + start.x * dy/dx) / (dy/dx)
					= (dim.y - start.y) * dx / dy + start.x
					*/

					ipoint idimm = surf->dim - 1;
					int c = start.y - start.x * d.y / d.x, // xlo
						ic = start.x - start.y * d.x / d.y, // ylo
						xhi = idimm.x * d.y / d.x + c,
						yhi = idimm.y * d.x / d.y + ic;
					int osx = start.x;

					if (start_in == false)
					{
						if (c < 0)
							start = ipoint(ic, 0);
						else if (c < idimm.y)
							start = ipoint(0, c);
						else
							start = ipoint(yhi, idimm.y);
					}

					if (end_in == false)
					{
						if (start_in == false && (start.x < osx || start.x >= end.x))
							return;

						if (xhi < 0)
							end = ipoint(ic, 0);
						else if (xhi < idimm.y)
							end = ipoint(idimm.x, xhi);
						else
							end = ipoint(yhi, idimm.y);
					}

					d = end - start;
				}
				else if (start_in == false || end_in == false)
					return;

				// super-optimized and fixed "Bresenham's line algorithm"

				bool slope = abs(d.y) > d.x;

				ipoint step;

				if (slope)
				{
					std::swap(start.x, start.y);
					std::swap(end.x, end.y);

					if (start.x > end.x)
						std::swap(start, end);

					d = end - start;
					step = ipoint(idim.x, 1);
				}
				else
					step = ipoint(1, idim.x);

				color_t* px = surf->buffer + dot(start, step);

				step.y *= get_sign(d.y);
				d.y = abs(d.y);

				int err = d.x;
				d <<= 1;

				for (int x = start.x; x < end.x; x++, px += step.x)
				{
					*px = color;

					err -= d.y;
					if (err <= 0)
					{
						px += step.y;
						err += d.x;
					}
				}
			}

			inline void triangle(ipoint a, ipoint b, ipoint c, color_t color, surface* surf)
			{
				line(a, b, color, surf);
				line(b, c, color, surf);
				line(a, c, color, surf);
			}

			inline void sure_x_line(int xs, int xb, int y, color_t color, surface* surf)
			{
				color_t* px = surf->buffer + y * surf->dim.x;
				for (int x = xs; x <= xb; x++)
					px[x] = color;
			}

			void rasterisation(ipoint a, ipoint b, ipoint c, color_t c1, color_t c2, surface* surf)
			{
				if (a.y > b.y)
					std::swap(a, b);
				if (b.y > c.y)
					std::swap(b, c);
				if (a.y > b.y)
					std::swap(a, b);

				ipoint dab = b - a,
					dbc = c - b,
					dac = c - a,
					idim(surf->dim.x - 1, surf->dim.y);

				int y = max(a.y, 0), sx, bx, t,
					byl = min(b.y, idim.y),
					cyl = min(c.y, idim.y);

				ipoint u1, u2;

				if (dac.x * dab.y > dab.x * dac.y)
					u1 = dac, u2 = dab;
				else
					u1 = dab, u2 = dac;

				while (y < byl)
				{
					t = y - a.y;
					sx = std::clamp(a.x + t * u2.x / u2.y, 0, idim.x);
					bx = std::clamp(a.x + t * u1.x / u1.y, 0, idim.x);

					sure_x_line(sx, bx, y++, c1, surf);
				}

				if (dac.x * dbc.y > dbc.x * dac.y)
					u1 = dac, u2 = dbc;
				else
					u1 = dbc, u2 = dac;

				while (y < cyl)
				{
					t = y - c.y;
					sx = std::clamp(c.x + t * u1.x / u1.y, 0, idim.x);
					bx = std::clamp(c.x + t * u2.x / u2.y, 0, idim.x);

					sure_x_line(sx, bx, y++, c2, surf);
				}
			}

			// I forgot how to sleep
			// FUCK
			void depth_sure_x_line(unsigned xs, unsigned xb, unsigned y, float z1, float z2, float* depth_buffer, color_t color, surface* surf)
			{
				unsigned offset = y * surf->dim.x;
				color_t* px = surf->buffer + offset;
				depth_buffer += offset;

				if (xs == xb)
				{
					if (xs != 0 && xs != surf->dim.x - 1 && depth_buffer[xs] > z1)
					{
						px[xs] = color;
						depth_buffer[xs] = z1;
					}
					return;
				}

				float z, t = (z2 - z1) / float(xb - xs);
				for (unsigned x = xs; x < xb; x++)
				{
					z = z1 + float(x - xs) * t;

					if (depth_buffer[x] > z)
					{
						px[x] = color;
						depth_buffer[x] = z;
					}
				}

				if (depth_buffer[xb] > z2 + EPSILON)
				{
					px[xb] = color;
					depth_buffer[xb] = z2;
				}
			}

			void depth_rasterisation(ipoint a, ipoint b, ipoint c, float azIn, float bzIn, float czIn, float* depth_buffer, color_t c1, color_t c2, surface* surf)
			{
				if (a.y > b.y)
				{
					std::swap(a, b);
					std::swap(azIn, bzIn);
				}

				if (b.y > c.y)
				{
					std::swap(b, c);
					std::swap(bzIn, czIn);
				}

				if (a.y > b.y)
				{
					std::swap(a, b);
					std::swap(azIn, bzIn);
				}

				ipoint dab = b - a,
					dbc = c - b,
					dac = c - a,
					idim(surf->dim.x - 1, surf->dim.y);

				int y = max(a.y, 0), t,
					byl = min(b.y, idim.y),
					cyl = min(c.y, idim.y);

				ipoint u1, u2;
				float uz1, uz2, ft;

				if (dac.x * dab.y < dab.x * dac.y)
				{
					u1 = dac, u2 = dab;
					uz1 = (czIn - azIn) / float(dac.y);
					uz2 = (bzIn - azIn) / float(dab.y);
				}
				else
				{
					u1 = dab, u2 = dac;
					uz1 = (bzIn - azIn) / float(dab.y);
					uz2 = (czIn - azIn) / float(dac.y);
				}

				while (y < byl)
				{
					t = y - a.y;
					ft = float(t);

					depth_sure_x_line(
						std::clamp(a.x + t * u1.x / u1.y, 0, idim.x),
						std::clamp(a.x + t * u2.x / u2.y, 0, idim.x),
						y++,
						azIn + ft * uz1,
						azIn + ft * uz2,
						depth_buffer, c1, surf
					);
				}

				if (dac.x * dbc.y > dbc.x * dac.y)
				{
					u1 = dac, u2 = dbc;
					uz1 = (azIn - czIn) / float(dac.y);
					uz2 = (bzIn - czIn) / float(dbc.y);
				}
				else
				{
					u1 = dbc, u2 = dac;
					uz1 = (bzIn - czIn) / float(dbc.y);
					uz2 = (azIn - czIn) / float(dac.y);
				}

				while (y < cyl)
				{
					t = y - c.y;
					ft = float(t);

					depth_sure_x_line(
						std::clamp(c.x + t * u1.x / u1.y, 0, idim.x),
						std::clamp(c.x + t * u2.x / u2.y, 0, idim.x),
						y++,
						czIn - ft * uz1,
						czIn - ft * uz2,
						depth_buffer, c2, surf
					);
				}
			}

			void depth_line(ipoint start, ipoint end, float* depth_buffer, color_t color, surface* surf)
			{
				ipoint d = end - start;

				if (d.y == 0)
				{
					if (d.x != 0)
						straight_line(start.x, end.x, start.y, false, color, surf);
					return;
				}

				if (d.x == 0)
				{
					if (d.y != 0)
						straight_line(start.y, end.y, start.x, true, color, surf);
					return;
				}

				if (start.x > end.x)
					std::swap(start, end), d = -d;

				ipoint idim = surf->dim;
				bool start_in = is_inside(start, surf->dim), end_in = is_inside(end, surf->dim);

				/*
				FIXED (13/3/24 11.41pm) Not sure
				BUG: (12/4/24 05.18am)
				when slope is bigger than 45 degrees and both points out from surface
				dx == |dx|
				|dy| > dx
				something weird happens:
				code is painting staring point but not painting end point
				but we are clamping both points by window dimension
				when start.x become bigger than dimension.x
				we set start.x to dimension.x
				but thats makes overflow then a single pixel come out from other side of window
				but this is a little problem I cant give even a bit computer power to fix this

				bug condutions:
				|dy| > |dx|
				start.y < 0
				end.x > dim.x

				HOWEVER its does not overflow whole buffer its just overflowing that line of y-axis
				except for only 1 position (in theory, I didnt test it)
				*/

				/*
				f(x) = ax + c
				f^-1(y) = (y - c) / a
				a = dy/dx
				c = y - ax
				c = start.y - start.x * dy/dx

				(-c/a, 0) = (f^-1(0), 0)

				(f^-1(dim.y), dim.y)
				f^-1(dim.y) = (dim.y - c) / a
				= (dim.y - start.y + start.x * dy/dx) / (dy/dx)
				= (dim.y - start.y) * dx / dy + start.x
				*/

				ipoint idimm = surf->dim - 1;
				int c = start.y - start.x * d.y / d.x, // xlo
					ic = start.x - start.y * d.x / d.y, // ylo
					xhi = idimm.x * d.y / d.x + c,
					yhi = idimm.y * d.x / d.y + ic;
				int osx = start.x;

				if (start_in == false)
				{
					if (c < 0)
						start = ipoint(ic, 0);
					else if (c < idimm.y)
						start = ipoint(0, c);
					else
						start = ipoint(yhi, idimm.y);
				}

				if (end_in == false)
				{
					if (start_in == false && (start.x < osx || start.x >= end.x))
						return;

					if (xhi < 0)
						end = ipoint(ic, 0);
					else if (xhi < idimm.y)
						end = ipoint(idimm.x, xhi);
					else
						end = ipoint(yhi, idimm.y);
				}

				d = end - start;

				// super-optimized and fixed "Bresenham's line algorithm"

				bool slope = abs(d.y) > d.x;

				ipoint step;

				if (slope)
				{
					std::swap(start.x, start.y);
					std::swap(end.x, end.y);

					if (start.x > end.x)
						std::swap(start, end);

					d = end - start;
					step = ipoint(idim.x, 1);
				}
				else
					step = ipoint(1, idim.x);

				int offset = dot(start, step);
				color_t* px = surf->buffer + offset;
				float* z = depth_buffer + offset;

				step.y *= get_sign(d.y);
				d.y = abs(d.y);

				int err = d.x;
				d <<= 1;

				for (int x = start.x; x < end.x; x++, px += step.x, z += step.x)
				{
					*px = color;
					*z = 0.0f;

					err -= d.y;
					if (err <= 0)
					{
						px += step.y;
						z += step.y;
						err += d.x;
					}
				}
			}

			inline void triangle(ipoint a, ipoint b, ipoint c, float* depth_buffer, color_t color, surface* surf)
			{
				depth_line(a, b, depth_buffer, color, surf);
				depth_line(b, c, depth_buffer, color, surf);
				depth_line(a, c, depth_buffer, color, surf);
			}
			
			void circle(ipoint center, int radius, color_t color, surface* surf)
			{
				point diff = { 0, radius };
				int d = 3 - (radius << 1);

				while (diff.y >= diff.x)
				{
					set_pixel(upoint( center.x - diff.x, center.y - diff.y ), color, surf);
					set_pixel(upoint( center.x - diff.x, center.y + diff.y ), color, surf);
					set_pixel(upoint( center.x + diff.x, center.y - diff.y ), color, surf);
					set_pixel(upoint( center.x + diff.x, center.y + diff.y ), color, surf);

					set_pixel(upoint( center.x - diff.y, center.y - diff.x ), color, surf);
					set_pixel(upoint( center.x - diff.y, center.y + diff.x ), color, surf);
					set_pixel(upoint( center.x + diff.y, center.y - diff.x ), color, surf);
					set_pixel(upoint( center.x + diff.y, center.y + diff.x ), color, surf);

					if (d < 0)
						d += (diff.x++ << 2) + 6;
					else
						d += 4 * (diff.x++ - diff.y--) + 10;
				}
			}
		}
	}
}
