/* window.h */

#ifndef XCB_WINDOW_H
#define XCB_WINDOW_H

#include "rendering/mvk_core.h"

// typedef xcb_connection_t xcb_connection_t;
// typedef xcb_screen_t xcb_screen_t;
// typedef xcb_window_t xcb_window_t;
// typedef xcb_intern_atom_reply_t xcb_intern_atom_reply_t;

typedef struct
{
	xcb_connection_t *connection;
	xcb_screen_t *xcb_screen;
	xcb_window_t window;
	xcb_intern_atom_reply_t *xcb_atom_window_reply;
	bool shouldExit;
} mxcb_window_info;

int mxcb_init_window(mxcb_window_info *mcxbWindowInfo, int surfaceSizeX, int surfaceSizeY);

int mxcb_update_window(mxcb_window_info *mcxbWindowInfo);

void mxcb_destroy_window(mxcb_window_info *mcxbWindowInfo);

// /* -----------------------------------------------------
// This source code is public domain ( CC0 )
// The code is provided as-is without limitations, requirements and responsibilities.
// Creators and contributors to this source code are provided as a token of appreciation
// and no one associated with this source code can be held responsible for any possible
// damages or losses of any kind.

// Original file creator:  Niko Kauppi (Code maintenance)
// Contributors:
// Teagan Chouinard (GLFW)
// ----------------------------------------------------- */

// #pragma once

// #include <string>

// class Renderer;

// class Window
// {
// public:
// 	Window( Renderer * renderer, uint32_t size_x, uint32_t size_y, std::string name );
// 	~Window();

// 	void Close();
// 	bool Update();

// private:
// 	void								_InitOSWindow();
// 	void								_DeInitOSWindow();
// 	void								_UpdateOSWindow();
// 	void								_InitOSSurface();

// 	void								_InitSurface();
// 	void								_DeInitSurface();

// 	Renderer						*	_renderer						= nullptr;

// 	VkSurfaceKHR						_surface						= VK_NULL_HANDLE;

// 	uint32_t							_surface_size_x					= 512;
// 	uint32_t							_surface_size_y					= 512;
// 	std::string							_window_name;

// 	VkSurfaceFormatKHR					_surface_format					= {};
// 	VkSurfaceCapabilitiesKHR			_surface_capabilities			= {};

// 	bool								_window_should_run				= true;

// #if USE_FRAMEWORK_GLFW
// 	GLFWwindow						*	_glfw_window					= nullptr;
// #elif VK_USE_PLATFORM_WIN32_KHR
// 	HINSTANCE							_win32_instance					= NULL;
// 	HWND								_win32_window					= NULL;
// 	std::string							_win32_class_name;
// 	static uint64_t						_win32_class_id_counter;
// #elif VK_USE_PLATFORM_XCB_KHR
// 	xcb_connection_t				*	mwinfo->xcb_connection					= nullptr;
// 	xcb_screen_t					*	_xcb_screen						= nullptr;
// 	xcb_window_t						_xcb_window						= 0;
// 	xcb_intern_atom_reply_t			*	_xcb_atom_window_reply			= nullptr;
// #endif
// };

#endif // XCB_WINDOW_H