#pragma once

#include "EBG.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <fstream>

/*
TODO:
ok  Mesh rot, local_rotation_center
ok  Rotation Matrix
ok  Keyboard & Mouse inputs
?  Camera pos, rot
ok Cache System (for cos & sin)

half done - Near & Far limits

********* ********* *********
Most Important:  Cut behind check
********* ********* *********

ok  depth buffer
ok  Rasterisation
ok Triangle normals
ok Normal check to see if visible

NOTE: used depth buffer - Z-axis sort (or any other way)
:  Cache System 3 (for Z-axis sort)

ok Mesh pos
ok Cache System 4 (for mesh vertex + pos)
*/

// 2*pi
#define M_2PI 6.283185307179586
// 1/(2*pi)
#define M_1_2PI 0.1591549430918953
// pi/3
#define M_PI_3 1.047197551196598

#undef near
#undef far

namespace eb3d
{
	using namespace ebg;

	typedef vec3<float> vertex_t;
	typedef unsigned short index16_t;

	namespace basic_math
	{
		// this is slower (than x * 2^p) if p is a constant
		inline constexpr float mulby2power(float x, short p)
		{
			long t = std::bit_cast<long, float>(x) + (p << 23);
			return std::bit_cast<float, long>(t);
		}

		inline constexpr float to2powerf(short p)
		{
			long t = 0x3f800000 + (p << 23);
			return std::bit_cast<float, long>(t);
		}

		inline constexpr double to2power(int p)
		{
			long long t = 0x3ff0000000000000 + (static_cast<long long>(p) << 52);
			return std::bit_cast<double, long long>(t);
		}

		inline constexpr float to_radians(float degree)
		{
			return degree * M_PI / 180.0f;
		}
	}

	namespace sincos
	{
		float* sin_hash_table, multipler, inv_multipler;
		unsigned resolution, res_4;

		void init(unsigned short log2_resolution)
		{
			resolution = 1 << log2_resolution;
			res_4 = resolution >> 2;

			sin_hash_table = TYPE_MALLOC(float, resolution);
			assert(sin_hash_table != nullptr);

			multipler = basic_math::mulby2power(M_1_2PI, log2_resolution);
			inv_multipler = basic_math::mulby2power(M_2PI, -log2_resolution);

			for (unsigned i = 0; i < resolution; i++)
				sin_hash_table[i] = sinf(static_cast<float>(i) * inv_multipler);
		}

		inline float sinf(float x)
		{
			return sin_hash_table[static_cast<unsigned>(x * multipler) % resolution];
		}

		inline float cosf(float x)
		{
			return sin_hash_table[(static_cast<unsigned>(x * multipler) + res_4) % resolution];
		}

		inline void sincosf(float x, float& sinv, float& cosv)
		{
			unsigned t = static_cast<unsigned>(x * multipler);
			sinv = sin_hash_table[t % resolution];
			cosv = sin_hash_table[(t + res_4) % resolution];
		}
	}

	// EULAR -ZYX
	struct reverse_inverse_rotation_data
	{
		fvec3 rotation;
		fvec3 sinv, cosv;
		float c[9];
		float t[4];

		void update()
		{
			sincos::sincosf(-rotation.x, sinv.x, cosv.x);
			sincos::sincosf(-rotation.y, sinv.y, cosv.y);
			sincos::sincosf(-rotation.z, sinv.z, cosv.z);

			t[0] = cosv.x * sinv.z;
			t[1] = sinv.x * cosv.z;
			t[2] = cosv.x * cosv.z;
			t[3] = sinv.x * sinv.z;

			c[0] = cosv.y * cosv.z;
			c[1] = cosv.y * sinv.z; // Negative
			c[2] = sinv.y;
			c[3] = t[0] + t[1] * sinv.y;
			c[4] = t[2] - t[3] * sinv.y;
			c[5] = sinv.x * cosv.y; // Negative
			c[6] = t[3] - t[2] * sinv.y;
			c[7] = t[0] * sinv.y + t[1];
			c[8] = cosv.x * cosv.y;
		}

		fvec3 rotate_vertex(fvec3 pos) const
		{
			return {
				c[0] * pos.x -
				c[1] * pos.y +
				c[2] * pos.z,
				c[3] * pos.x +
				c[4] * pos.y -
				c[5] * pos.z,
				c[6] * pos.x +
				c[7] * pos.y +
				c[8] * pos.z
			};
		}

		inline fvec3 normal_forward() const
		{
			return {
				t[3] - t[2] * sinv.y,
				t[1] + t[0] * sinv.y,
				c[8]
			};
		}

		inline fvec3 normal_right() const
		{
			return {
				c[0],
				c[1],
				sinv.y
			};
		}
	};

	// EULAR XYZ
	struct rotation_data
	{
		fvec3 rotation;
		fvec3 sinv, cosv;
		float c[9];
		float t[4];

		fvec3 old_rotate_vertex(fvec3 pos) const
		{
			float t = pos.y;
			pos.y = cosv.x * t - sinv.x * pos.z;
			pos.z = sinv.x * t + cosv.x * pos.z;

			t = pos.x;
			pos.x = cosv.y * t + sinv.y * pos.z;
			pos.z = cosv.y * pos.z - sinv.y * t;

			t = pos.x;
			pos.x = cosv.z * t - sinv.z * pos.y;
			pos.y = sinv.z * t + cosv.z * pos.y;

			return pos;
		}

		void update()
		{
			sincos::sincosf(rotation.x, sinv.x, cosv.x);
			sincos::sincosf(rotation.y, sinv.y, cosv.y);
			sincos::sincosf(rotation.z, sinv.z, cosv.z);

			t[0] = sinv.x * sinv.z;
			t[1] = cosv.x * cosv.z;
			t[2] = cosv.x * sinv.z;
			t[3] = sinv.x * cosv.z;

			c[0] = cosv.y * cosv.z;
			c[1] = sinv.y * t[3] - t[2];
			c[2] = sinv.y * t[1] + t[0];
			c[3] = sinv.z * cosv.y;
			c[4] = sinv.y * t[0] + t[1];
			c[5] = t[2] * sinv.y - t[3];
			c[6] = sinv.y;
			c[7] = cosv.y * sinv.x;
			c[8] = cosv.y * cosv.x;
		}

		fvec3 rotate_vertex(fvec3 pos) const
		{
			return {
				c[0] * pos.x +
				c[1] * pos.y +
				c[2] * pos.z,
				c[3] * pos.x +
				c[4] * pos.y +
				c[5] * pos.z,
				c[7] * pos.y -
				c[6] * pos.x +
				c[8] * pos.z
			};
		}

		inline fvec3 forward() const
		{
			return {
				c[2],
				c[5],
				c[8]
			};
		}

		inline fvec3 right() const
		{
			return {
				c[0],
				c[3],
				-c[6]
			};
		}

		/*
		fvec3 rotate_vertex(fvec3 pos) const
		{
			float t1 = sinv.z * sinv.y;
			float t2 = cosv.z * sinv.y;

			float c6 = t1		* sinv.x	+ cosv.z * cosv.x;
			float c4 = cosv.y	* sinv.x;
			float c2 = t2		* sinv.x	+ sinv.z * cosv.x;

			float c7 = t1		* cosv.x	- cosv.z * sinv.x;
			float c3 = t2		* cosv.x	- sinv.z * sinv.x;

			float c1 = cosv.z	* cosv.y;
			float c5 = sinv.z	* cosv.y;

			return {
				c1 * pos.x +
				c2 * pos.y * pos.y +
				c3 * pos.z,

				c5 * pos.x +
				c6 * pos.y +
				c7 * pos.z,

				c4 * pos.y +
				c1 * pos.z -
				sinv.y * pos.x
			};
		}
		*/

		/*
		fvec3 inverse_rotate_vertex(fvec3 pos) const
		{
			float t = pos.y;
			pos.y = cosv.x * t + sinv.x * pos.z;
			pos.z = cosv.x * pos.z - sinv.x * t;

			t = pos.x;
			pos.x = cosv.y * t - sinv.y * pos.z;
			pos.z = cosv.y * pos.z + sinv.y * t;

			t = pos.x;
			pos.x = cosv.z * t + sinv.z * pos.y;
			pos.y = cosv.z * pos.y - sinv.z * t;

			return pos;
		}
		*/
	};

	struct triangle
	{
		// indexes of vertices
		index16_t a, b, c;
		// fvec3 normal; // TODO: add triangle center ??
		float inv_normal_length;
		// TODO: normals
	};

	class compound_mesh;

	struct camera
	{
		const float xfov_2, h, near;
		fvec3 position, forward, right;
		reverse_inverse_rotation_data rotation;

		inline void update()
		{
			rotation.update();
			forward = rotation.normal_forward();
			right = rotation.normal_right();
		}

		camera(float xfov_2, float near, fvec3 position, fvec3 rotation) : xfov_2(xfov_2), h(tanf(M_PI_2 - xfov_2)), near(near), position(position)
		{
			this->rotation.rotation = rotation;
			update();
		}

		inline fvec3 persfz(vertex_t v) const
		{
			// v = rotation.inverse_rotate_vertex(v - position);

			float t = h / v.z;
			return fvec3(v.x * t, v.y * t, t);
		}

		inline fvec2 persf(vertex_t v) const
		{
			// v = rotation.inverse_rotate_vertex(v - position);

			float t = h / v.z;
			return fvec2(v.x * t, v.y * t);
		}

		void draw_triangle(compound_mesh* mesh, index16_t index, basic_engine* engine) const;
	};

	/*
	class static_mesh
	{
	public:
		vertex_t* world_vertices;
		unsigned vertex_amount;
		triangle* triangles;
		unsigned triangle_amount;

		static_mesh(unsigned vertex_amount, unsigned triangle_amount)
			: vertex_amount(vertex_amount), triangle_amount(triangle_amount)
		{
			world_vertices = TYPE_MALLOC(vertex_t, vertex_amount);
			triangles = TYPE_MALLOC(triangle, triangle_amount);
		}

		static_mesh(const char* file_name, bool dynamic = false);

		void draw_normals(camera* cam, basic_engine* engine);

		void calc_normals()
		{
			for (int i = 0; i < triangle_amount; i++)
			{
				vertex_t a = world_vertices[triangles[i].a];
				triangles[i].normal = cross(world_vertices[triangles[i].b] - a, world_vertices[triangles[i].c] - a) / triangles[i].normal_length;
			}
		}

		// aka: calc_normals_and_lengths & calc_normals
		void static_calc_normals()
		{
			for (int i = 0; i < triangle_amount; i++)
			{
				vertex_t a = world_vertices[triangles[i].a];
				triangles[i].normal = cross(world_vertices[triangles[i].b] - a, world_vertices[triangles[i].c] - a);
				triangles[i].normal_length = magnitude(triangles[i].normal);
				triangles[i].normal /= triangles[i].normal_length;
			}
		}
	};
	*/

	enum update_types
	{
		tauto = 0,
		tstatic = 1,
		tdynamic = 2,
		tonly_pos = 3
	};

	struct basic_color_conversation_data
	{
		float rm, rc,
			gm, gc,
			bm, bc;
	};

	class compound_mesh
	{
	public:
		vertex_t* world_vertices;
		unsigned vertex_amount;
		triangle* triangles;
		unsigned triangle_amount;
		basic_color_conversation_data bccd;

		vertex_t* local_vertices;

		fvec3 position;
		rotation_data* rotation;

		bool is_static;

		inline void static_calc_world_vertices(camera* cam)
		{
			for (unsigned i = 0; i < vertex_amount; i++)
				world_vertices[i] = cam->rotation.rotate_vertex(local_vertices[i] - cam->position);
		}

		inline void only_pos_calc_world_vertices(camera* cam)
		{
			fvec3 temp = position - cam->position;
			for (unsigned i = 0; i < vertex_amount; i++)
				world_vertices[i] = cam->rotation.rotate_vertex(local_vertices[i] + temp);
		}

		inline void calc_world_vertices(camera* cam)
		{
			fvec3 temp = position - cam->position;
			for (unsigned i = 0; i < vertex_amount; i++)
				world_vertices[i] = cam->rotation.rotate_vertex(rotation->rotate_vertex(local_vertices[i]) + temp);
		}

		inline void update(camera* cam, uint8_t update_type = tauto)
		{
			switch (update_type)
			{
			case tauto:
				if (is_static)
				{
			case tstatic:
				static_calc_world_vertices(cam);
				return;
				}
			case tdynamic:
				rotation->update();
				calc_world_vertices(cam);
				return;
			case tonly_pos:
				only_pos_calc_world_vertices(cam);
				return;
			}
		}

		void draw(camera* cam, basic_engine* engine);

		void calc_normal_lengths()
		{
			for (int i = 0; i < triangle_amount; i++)
			{
				vertex_t a = local_vertices[triangles[i].a];
				triangles[i].inv_normal_length = 1.0f / magnitude(cross(local_vertices[triangles[i].b] - a, local_vertices[triangles[i].c] - a));
			}
		}

		void setup(camera* cam)
		{
			update(cam);
			calc_normal_lengths();
		}

		compound_mesh(unsigned vertex_amount, unsigned triangle_amount)
			: vertex_amount(vertex_amount), triangle_amount(triangle_amount), position(position)
		{
			world_vertices = TYPE_MALLOC(vertex_t, vertex_amount);
			triangles = TYPE_MALLOC(triangle, triangle_amount);
			local_vertices = TYPE_MALLOC(vertex_t, vertex_amount);

			is_static = true;
		}

		compound_mesh(unsigned vertex_amount, unsigned triangle_amount, fvec3 position, fvec3 rotation)
			: vertex_amount(vertex_amount), triangle_amount(triangle_amount), position(position)
		{
			world_vertices = TYPE_MALLOC(vertex_t, vertex_amount);
			triangles = TYPE_MALLOC(triangle, triangle_amount);
			local_vertices = TYPE_MALLOC(vertex_t, vertex_amount);

			this->rotation = new rotation_data;
			this->rotation->rotation = rotation;

			is_static = false;
		}

		compound_mesh(const char* file_name, fvec3 size = 1.0f)
		{
			is_static = true;

			std::ifstream file(file_name);
			assert(file.is_open());

			char* buffer = reinterpret_cast<char*>(data::cb);

			index16_t i = 0;
			unsigned amount;
			char* start;
			triangle tri;
			vertex_t v;

			vertex_amount = 0;
			triangle_amount = 0;
			world_vertices = nullptr;
			triangles = nullptr;

			while (file.eof() == false)
			{
				file.getline(buffer, data::cb_size);

				switch (buffer[0])
				{
				case 'a':
					amount = atoi(&buffer[4]);
					if (buffer[2] == 'v')
						vertex_amount = amount;
					else if (buffer[2] == 'f')
						triangle_amount = amount;
					break;
				case 'A':
					if (vertex_amount != 0)
						break;
					while (file.eof() == false)
					{
						file.getline(buffer, 128);
						switch (buffer[0])
						{
						case 'v':
							vertex_amount++;
							break;
						case 'f':
							triangle_amount++;
							break;
						}
					}
					file.clear();
					file.seekg(0);
				case 'm':
					world_vertices = TYPE_MALLOC(vertex_t, vertex_amount);
					local_vertices = TYPE_MALLOC(vertex_t, vertex_amount);
					triangles = TYPE_MALLOC(triangle, triangle_amount);
					break;
				case 'c':
					i = 0;
					break;
				case 'v':
					start = buffer + 2;
					v.x = atof(start);

					while (*start != ' ') start++;
					start++;
					v.y = atof(start);

					while (*start != ' ') start++;
					start++;
					v.z = atof(start);

					local_vertices[i] = v * size;
					i++;
					break;
				case 'f':
					start = buffer + 2;
					tri.a = atol(start) - 1;

					while (*start != ' ') start++;
					start++;
					tri.b = atol(start) - 1;

					while (*start != ' ') start++;
					start++;
					tri.c = atol(start) - 1;

					triangles[i] = tri;
					i++;
					break;
				}
			}
		}

		compound_mesh(const char* file_name, fvec3 position, fvec3 rotation, fvec3 size = 1.0f) : compound_mesh(file_name, size)
		{
			this->position = position;
			is_static = false;
			this->rotation = new rotation_data;
			this->rotation->rotation = rotation;
		}
	};

	inline ipoint mapto_engine(fpoint p, basic_engine* engine)
	{
		return ipoint(
			int(p.x * engine->fhdim.x) + engine->hdim.x,
			int(p.y * engine->fhdim.x) + engine->hdim.y
		);
	}

	inline void clip_1i_2o_triangle(vertex_t* vertices, char i, char o1, char o2, float near)
	{
		// x * (dz / dx) + iz - ix * (dz / dx) = near
		// y * (dz / dy) + iz - ix * (dz / dy) = near
		// T = (near - iz) * (dT / dz) + ix
		// T = n_iz * dT * izN + iT
		vertex_t inV = vertices[i];

		fvec3 d1 = vertices[o1] - inV, d2 = vertices[o2] - inV;

		vertices[o1].z = near;
		vertices[o2].z = near;

		float
			n_izm1 = (near - inV.z) / d1.z,
			n_izm2 = (near - inV.z) / d2.z;

		vertices[o1].x = inV.x + d1.x * n_izm1;
		vertices[o1].y = inV.y + d1.y * n_izm1;
		vertices[o2].x = inV.x + d2.x * n_izm2;
		vertices[o2].y = inV.y + d2.y * n_izm2;
	}

	inline void clip_2i_1o_triangle(vertex_t* vertices, char i1, char i2, char o, float near)
	{
		vertex_t outV = vertices[o];

		fvec3 d1 = vertices[i1] - outV, d2 = vertices[i2] - outV;

		float
			n_ozm1 = (near - outV.z) / d1.z,
			n_ozm2 = (near - outV.z) / d2.z;

		vertices[o] = { outV.x + d1.x * n_ozm1, outV.y + d1.y * n_ozm1, near };
		vertices[3] = { outV.x + d2.x * n_ozm2, outV.y + d2.y * n_ozm2, near };
	}

	void camera::draw_triangle(compound_mesh* mesh, index16_t index, basic_engine* engine) const
	{
		triangle tri = mesh->triangles[index];

		vertex_t vertices[4] = {
			mesh->world_vertices[tri.a],
			mesh->world_vertices[tri.b],
			mesh->world_vertices[tri.c]
		};

		vertex_t normal = cross(vertices[1] - vertices[0], vertices[2] - vertices[0]);
		if (dot(normal, vertices[0]) >= 0.0f)
			return;

		char iV[3], oV[3], iI = 0, oI = 0, t;

		// temporary bug fix
		float some_value = near + magnitude(vertices[0]) * EPSILON;
		(vertices[0].z < some_value ? oV[oI++] : iV[iI++]) = 0;
		(vertices[1].z < some_value ? oV[oI++] : iV[iI++]) = 1;
		(vertices[2].z < some_value ? oV[oI++] : iV[iI++]) = 2;

		if (oI == 3)
			return;

		// normal *= tri.inv_normal_length;

		// float lightning = dot(normal, { 0.0f, 0.0f, -1.0f }) * 255.0f;

		// Camera independent lighting
		float lightning =
			dot(
				cross(
					mesh->local_vertices[tri.b] - mesh->local_vertices[tri.a],
					mesh->local_vertices[tri.c] - mesh->local_vertices[tri.a]
				),
				normalize({ -1.0f, 1.0f, -1.0f })
			)
			* tri.inv_normal_length;
		color_t color = RGB(
			uint8_t(max((lightning * mesh->bccd.bm + mesh->bccd.bc) * 255.0f, 0.0f)),
			uint8_t(max((lightning * mesh->bccd.gm + mesh->bccd.gc) * 255.0f, 0.0f)),
			uint8_t(max((lightning * mesh->bccd.rm + mesh->bccd.rc) * 255.0f, 0.0f))
		);

		ipoint mappedv[3];

		if (oI == 2)
		{
			clip_1i_2o_triangle(vertices, iV[0], oV[0], oV[1], some_value);
		}
		else if (oI == 1)
		{
			iI = iV[1];
			t = oV[0];

			clip_2i_1o_triangle(vertices, iV[0], iI, t, some_value);
		}

		mappedv[0] = mapto_engine(persf(vertices[0]), engine);
		mappedv[1] = mapto_engine(persf(vertices[1]), engine); // Must be stored in cache
		mappedv[2] = mapto_engine(persf(vertices[2]), engine);

		// graphics::draw::triangle(mappedv[0], mappedv[1], mappedv[2], engine->depth_buffer, colors::white, &engine->surface);

		graphics::draw::depth_rasterisation(
			mappedv[0], mappedv[1], mappedv[2],
			vertices[0].z, vertices[1].z, vertices[2].z,
			engine->depth_buffer, color, color, &engine->surface
		);

		if (oI == 1)
		{
			// graphics::draw::triangle(mappedv[0], mappedv[1], mappedv[2], engine->depth_buffer, colors::white, &engine->surface);

			graphics::draw::depth_rasterisation(
				mapto_engine(persf(vertices[3]), engine),
				mappedv[iI],
				mappedv[t],
				vertices[3].z,
				vertices[iI].z,
				vertices[t].z,
				engine->depth_buffer, color, color, &engine->surface
			);
		}
	}

	inline void compound_mesh::draw(camera* cam, basic_engine* engine)
	{
		for (int i = 0; i < triangle_amount; i++)
			cam->draw_triangle(this, i, engine);
	}

	struct sphere_collision_module
	{
		fvec3* orianted_position;
		float radius;
	};

	/*
	// debug tool
	void static_mesh::draw_normals(camera* cam, basic_engine* engine)
	{
		for (unsigned i = 0; i < triangle_amount; i++)
		{
			fvec3 triangle_center = (world_vertices[triangles[i].a] + world_vertices[triangles[i].b] + world_vertices[triangles[i].c]) / 3.0f;

			graphics::draw::line(
				mapto_engine(cam->persf(triangle_center), engine),
				mapto_engine(cam->persf(triangle_center + triangles[i].normal * 0.25f), engine),
				colors::green, &engine->surface
			);
		}
	}
	*/
}
