#pragma once

#include <Windows.h>
#include <iostream>

#pragma comment(lib, "winmm.lib")

#include "EBG_graphics.h"

std::string GetLastErrorAsString()
{
	//Get the error message ID, if any.
	DWORD errorMessageID = ::GetLastError();
	if (errorMessageID == 0) {
		return std::string(); //No error message has been recorded
	}

	LPSTR messageBuffer = nullptr;

	//Ask Win32 to give us the string version of that message ID.
	//The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
	size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

	//Copy the error message into a std::string.
	std::string message(messageBuffer, size);

	//Free the Win32's string's buffer.
	LocalFree(messageBuffer);

	return message;
}

namespace ebg
{
	struct mouse_s
	{
		POINT win_pos;
		ipoint pos, old_pos, delta;
		bool in_screen;
		bool left, right, middle;
		bool tleft, tright, tmiddle;

		inline bool update_clicks(UINT u_msg)
		{
			switch (u_msg)
			{
			case WM_LBUTTONDOWN:
				tleft = true;
				left = true;
				return true;
			case WM_LBUTTONUP:
				tleft = true;
				left = false;
				return true;
			case WM_RBUTTONDOWN:
				tright = true;
				right = true;
				return true;
			case WM_RBUTTONUP:
				tright = true;
				right = false;
				return true;
			case WM_MBUTTONDOWN:
				tmiddle = true;
				middle = true;
				return true;
			case WM_MBUTTONUP:
				tmiddle = true;
				middle = false;
				return true;
			default:
				return false;
			}
		}
	};

	struct abc_keyboard
	{
		bool keys[26];

		inline bool get_key(char c) const
		{
			return keys[c - 'a'];
		}

		inline bool update_key_down(WPARAM w_param)
		{
			if (w_param >= 'A' && w_param <= 'Z')
			{
				keys[w_param - 'A'] = true;
				return true;
			}
			return false;
		}

		inline bool update_key_up(WPARAM w_param)
		{
			if (w_param >= 'A' && w_param <= 'Z')
			{
				keys[w_param - 'A'] = false;
				return true;
			}
			return false;
		}
	};

	constexpr float dt_multipler = 1000.0f, inv_dt_multipler = 0.001f;
	constexpr ipoint extra_size(16, 39);

	struct win_data
	{
		BITMAPINFO bitmap_info;
		WNDCLASSA wndc;
		HDC hdc;
		FILE* console;
		MSG msg;
	};

	struct basic_engine
	{
		fpoint fdim, fhdim;
		ipoint idim;
		upoint hdim;

		float ratio, inv_ratio;

		bool running;
		unsigned target_fps;
		float delta_time;

		mouse_s mouse;
		abc_keyboard keyboard;
		float* depth_buffer;
		graphics::surface surface;
		HWND window;

		win_data data;

		bool has_console;
		float target_delta_time;
		unsigned target_frame_time, tick, real_dt, start_time;
		// unsigned delta time
		unsigned udt;

		void update_mouse()
		{
			GetCursorPos(&mouse.win_pos);
			ScreenToClient(window, &mouse.win_pos);
			mouse.old_pos = mouse.pos;
			mouse.pos = ipoint(mouse.win_pos.x, surface.dim.y - mouse.win_pos.y);
			mouse.delta = mouse.pos - mouse.old_pos;
			mouse.in_screen = is_inside(mouse.pos, surface.dim);
		}

		inline void refresh_mouse_ticks()
		{
			mouse.tleft = false;
			mouse.tmiddle = false;
			mouse.tright = false;
		}

		// auto updates mouse
		void start_tick()
		{
			start_time = timeGetTime();

			while (PeekMessageW(&data.msg, window, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&data.msg);
				DispatchMessageW(&data.msg);
			}

			update_mouse();
		}

		void end_tick()
		{
			StretchDIBits(
				data.hdc,
				0, 0, idim.x, idim.y,
				0, 0, idim.x, idim.y,
				surface.buffer, &data.bitmap_info,
				DIB_RGB_COLORS, SRCCOPY
			);

			refresh_mouse_ticks();

			real_dt = timeGetTime() - start_time;
			if (real_dt < target_frame_time)
			{
				Sleep(target_frame_time - real_dt);
				udt = target_frame_time;
				delta_time = target_delta_time;
			}
			else
			{
				udt = real_dt;
				delta_time = static_cast<float>(udt) * inv_dt_multipler;
			}
			tick++;
		}

		basic_engine() {}

		basic_engine(const char* title, upoint window_dimension, bool console, int fps,
			WNDPROC event_handler, HINSTANCE hInstance, bool alloc_depth_buffer = false)
		{
			tick = real_dt = start_time = udt = 0L;
			delta_time = .0f;
			has_console = console;
			running = false;
			target_fps = fps;
			target_frame_time = 1000 / fps;
			target_delta_time = dt_multipler / static_cast<float>(fps);

			memset(keyboard.keys, 0, 26);
			memset(&mouse.left, 0, 6);

			mouse.pos = ipoint(0, 0);

			idim = window_dimension;
			hdim = window_dimension >> 1U;
			fdim = window_dimension;
			fhdim = fdim * 0.5f;

			ratio = fdim.x / fdim.y;
			inv_ratio = fdim.y / fdim.x;

			surface = graphics::surface(window_dimension);

			depth_buffer = alloc_depth_buffer == true ? TYPE_MALLOC(float, surface.buffer_size) : nullptr;

			window = nullptr;

			if (console)
			{
				AllocConsole();
				freopen_s(&data.console, "CONOUT$", "w", stdout);
			}

			data.wndc = { };

			data.wndc.lpfnWndProc = event_handler;
			data.wndc.hInstance = hInstance;
			data.wndc.lpszClassName = title;

			if (RegisterClassA(&data.wndc) == 0)
			{
				std::cout << GetLastErrorAsString();
				MessageBoxA(nullptr, "Failed to register Window Class!", "Error", MB_OK);
				return;
			}

			window = CreateWindowExA(
				0,
				data.wndc.lpszClassName,
				data.wndc.lpszClassName,
				WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX,
				CW_USEDEFAULT, CW_USEDEFAULT,
				idim.x + extra_size.x, idim.y + extra_size.y,
				nullptr, nullptr, hInstance, nullptr
			);

			if (window == nullptr)
			{
				UnregisterClassA(data.wndc.lpszClassName, hInstance);
				std::cout << GetLastErrorAsString();
				MessageBoxA(nullptr, "Failed to register Window Class!", "Error", MB_OK);
				return;
			}

			data.bitmap_info.bmiHeader.biSize = sizeof(data.bitmap_info.bmiHeader);
			data.bitmap_info.bmiHeader.biWidth = window_dimension.x;
			data.bitmap_info.bmiHeader.biHeight = window_dimension.y;
			data.bitmap_info.bmiHeader.biPlanes = 1;
			data.bitmap_info.bmiHeader.biBitCount = 32;
			data.bitmap_info.bmiHeader.biCompression = BI_RGB;

			data.hdc = GetDC(window);

			ShowWindow(window, 1);
			UpdateWindow(window);

			running = true;
		}
	};

	void delete_basic_engine(basic_engine* be)
	{
		be->running = false;

		if (be->has_console)
		{
			fclose(be->data.console);
			FreeConsole();
		}

		ReleaseDC(be->window, be->data.hdc);
		DestroyWindow(be->window);
		UnregisterClassA(be->data.wndc.lpszClassName, be->data.wndc.hInstance);

		graphics::delete_surface(&be->surface);
	}
}
