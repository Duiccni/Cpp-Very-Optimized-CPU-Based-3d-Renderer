#include "EBG.h"
#include "EBG_3d.h"

ebg::basic_engine beta;
ebg::upoint window_dimension(1920, 1080);

LRESULT CALLBACK window_proc(HWND hwnd, UINT u_msg, WPARAM w_param, LPARAM l_param)
{
	if (beta.mouse.update_clicks(u_msg))
		return 0;

	switch (u_msg)
	{
	case WM_CLOSE:
		beta.running = false;
		return 0;
	case WM_KEYDOWN:
		if (beta.keyboard.update_key_down(w_param))
			return 0;
		return 0;
	case WM_KEYUP:
		if (beta.keyboard.update_key_up(w_param))
			return 0;
		if (w_param == VK_ESCAPE)
			beta.running = false;
		return 0;
	default:
		return DefWindowProcA(hwnd, u_msg, w_param, l_param);
	}
}

#define Surface beta.surface

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd
)
{
	using namespace ebg;
	using namespace eb3d;

	data::init();
	sincos::init(12);

	beta = basic_engine("Test b1 3d", window_dimension, false, 50, window_proc, hInstance, true);

	camera cam(M_PI_3, EPSILON, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }/*, {0.0f, 0.0f, 0.0f}*/);

	/*
	dynamic_mesh cube(8, 12, { 0.0f, 0.0f, 3.0f }, { 0.0f, 0.0f, 0.0f });

	cube.local_vertices[0] = { -1.0f, -1.0f, -1.0f };
	cube.local_vertices[1] = { 1.0f, -1.0f, -1.0f };
	cube.local_vertices[2] = { -1.0f,  1.0f, -1.0f };
	cube.local_vertices[3] = { 1.0f,  1.0f, -1.0f };
	cube.local_vertices[4] = { -1.0f, -1.0f,  1.0f };
	cube.local_vertices[5] = { 1.0f, -1.0f,  1.0f };
	cube.local_vertices[6] = { -1.0f,  1.0f,  1.0f };
	cube.local_vertices[7] = { 1.0f,  1.0f,  1.0f };

	// F*CKING CLOCKWISE
	cube.triangles[0] = { 0, 2, 1 }; // Front
	cube.triangles[1] = { 3, 1, 2 };
	cube.triangles[2] = { 1, 5, 0 }; // Bottom
	cube.triangles[3] = { 4, 0, 5 };
	cube.triangles[4] = { 2, 6, 3 }; // Top
	cube.triangles[5] = { 6, 7, 3 };
	cube.triangles[6] = { 0, 4, 2 }; // Left
	cube.triangles[7] = { 4, 6, 2 };
	cube.triangles[8] = { 1, 3, 7 }; // Right
	cube.triangles[9] = { 5, 1, 7 };
	cube.triangles[10] = { 4, 5, 6 }; // Back
	cube.triangles[11] = { 7, 6, 5 };

	cube.setup();
	*/

	// dynamic_mesh teapot("utahTeapot.txt", { 0.0f, -1.5f, 7.0f }, { 0.0f, 0.0f, 0.0f });
	// teapot.setup();
	// dynamic_mesh stone("unk.txt", { 5.0f, -2.5f, 0.0f }, { 0.0f, -M_PI_4, 0.0f });
	// stone.setup();
	// dynamic_mesh thing("unk2.txt", { 0.0f, -6.0f, 20.0f }, { 0.0f, M_PI_3, 0.0f });
	// thing.setup();
	// dynamic_mesh ship("spaceship.txt", { 0.0f, -2.0f, 8.0f }, { 0.0f, M_PI_4, 0.0f });
	// ship.setup();

	compound_mesh landscape("lan.txt", {5.0f, 2.0f, 5.0f});
	landscape.setup(&cam);
	landscape.bccd.rm = 0.7f;
	landscape.bccd.rc = 0.0f;
	landscape.bccd.gm = 0.5f;
	landscape.bccd.gc = 0.5f;
	landscape.bccd.bm = -0.2f;
	landscape.bccd.bc = 0.5f;

	compound_mesh sphere("sphere.txt", { 0.0f, 3.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 5.0f);
	sphere.setup(&cam);
	sphere.bccd.rm = 1.0f;
	sphere.bccd.rc = 0.0f;
	sphere.bccd.gm = -0.1f;
	sphere.bccd.gc = 0.1f;
	sphere.bccd.bm = 1.0f;
	sphere.bccd.bc = 0.0f;

	compound_mesh sun("sphere.txt", { 1500.0f, 700.0f, 700.0f }, { 0.0f, 0.0f, 0.0f }, 1000.0f);
	sun.setup(&cam);
	sun.bccd.rm = 1.0f;
	sun.bccd.rc = 0.0f;
	sun.bccd.gm = 0.7f;
	sun.bccd.gc = 0.3f;
	sun.bccd.bm = 0.0f;
	sun.bccd.bc = 0.0f;

	compound_mesh unk3("unk3.txt", { 1.0f, 10.0f, 1.0f }, { 0, 0, 0 }, 4.0f);
	unk3.setup(&cam);
	unk3.bccd.rm = -1.0f;
	unk3.bccd.rc = 1.0f;
	unk3.bccd.gm = 0.7f;
	unk3.bccd.gc = 0.3f;
	unk3.bccd.bm = 0.7f;
	unk3.bccd.bc = 0.0f;

	sphere_collision_module collision;
	collision.orianted_position = &sphere.position;
	collision.radius = 1.0f;

	/*
	dynamic_mesh ship(3, 1, { 0.0f, -0.1f, 1.0f }, { 0.0f, 0.0f, 0.0f });

	ship.local_vertices[0] = { 0.0f, 0.0f, 0.0f };
	ship.local_vertices[1] = { 0.0f, 0.0f, 4.0f };
	ship.local_vertices[2] = { 0.1f, 0.0f, 5.0f };

	ship.triangles[0] = { 0, 1, 2 };
	ship.setup();
	*/

	while (beta.running)
	{
		beta.start_tick();

		/*
		float some_multipler = beta.delta_time * 0.001f;

		if (beta.keyboard.get_key('w'))
			cam.position += cam.forward * some_multipler;
		else if (beta.keyboard.get_key('s'))
			cam.position -= cam.forward * some_multipler;

		if (beta.mouse.right)
		{
			cam.rotation.rotation.y += static_cast<float>(beta.mouse.delta.x) * some_multipler;
			cam.rotation.rotation.x -= static_cast<float>(beta.mouse.delta.y) * some_multipler;
			cam.update();
		}
		*/

		0 >> Surface;

		// (float)0b01111111011111110111111101111111 = very large float number
		memset(beta.depth_buffer, 0b01111111, Surface.buffer_size << 2);

		/*
		cube.rotation.rotation += fvec3(0.01f, -0.01f, 0.02f);

		// local vertex to world vertex
		cube.update();

		// calculate screen position of vertex
		// cube.calc_surface_points(&cam, &beta);

		// draw only lines
		cube.draw(&cam, &Surface);

		// draw normals
		cube.draw_normals(&cam, &beta);
		*/

		/*
		char m = beta.keyboard.get_key('c') | (beta.keyboard.get_key('x') << 1);
		if (m)
		{
			if (m == 1)
				ship.rotation.rotation.y += 0.02f;
			else if (m == 0b10)
				ship.rotation.rotation.y -= 0.02f;

			ship.update();
		}
		*/

		if (beta.mouse.right)
		{
			cam.rotation.rotation.x -= float(beta.mouse.delta.y) * 0.005f;
			cam.rotation.rotation.y += float(beta.mouse.delta.x) * 0.005f;
		}
		cam.update();

		if (beta.keyboard.get_key('w'))
			cam.position += cam.forward * 0.2f;
		else if (beta.keyboard.get_key('s'))
			cam.position -= cam.forward * 0.2f;
		if (beta.keyboard.get_key('d'))
			cam.position += cam.right * 0.2f;
		else if (beta.keyboard.get_key('a'))
			cam.position -= cam.right * 0.2f;

		if (beta.keyboard.get_key('e'))
			cam.position.y += 0.1f;
		else if (beta.keyboard.get_key('q'))
			cam.position.y -= 0.1f;

		landscape.update(&cam);
		landscape.draw(&cam, &beta);

		sphere.update(&cam, tonly_pos);
		sphere.draw(&cam, &beta);

		sun.bccd.bc += 0.01f;

		sun.update(&cam, tonly_pos);
		sun.draw(&cam, &beta);

		unk3.update(&cam);
		unk3.draw(&cam, &beta);

		// teapot.rotation.rotation.y += 0.02f;
		// teapot.update();

		// ship.draw_normals(&cam, &beta);

		// teapot.draw(&cam, &beta);
		// stone.draw(&cam, &beta);
		// thing.draw(&cam, &beta);
		// ship.draw(&cam, &beta);

		beta.end_tick();

		char* buffer = reinterpret_cast<char*>(data::cb);
		strcpy_s(buffer, 14, "Performance: ");
		_itoa_s(beta.udt, buffer + 13, 128, 10);
		SetWindowTextA(beta.window, buffer);

		// print tick
		/*
		if (beta.has_console)
		{
			std::cout << "tick\n";
		}
		*/
	}

	delete_basic_engine(&beta);
	data::free_cb();

	return 0;
}