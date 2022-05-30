#pragma once
// Standard lib
#include <iostream>
#include <string>
#include <stdio.h>
// Random RanKIT
#include "random-rankit.h"
#include "random-windows.h"
// IMGUI - OPENGL GUI
#include "ImGUI/imgui.h"
#include "ImGUI/imgui_impl_glfw.h"
#include "ImGUI/imgui_impl_opengl3.h"
// OpenGL
#if defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GLEW/glew.h>
#endif 
#include <GLFW/glfw3.h>

#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

namespace Lightning
{
	namespace Front
	{
		enum class ELightMode
		{
			LIGHT_DIRECT,
			LIGHT_POINT,
			LIGHT_SPOTLIGHT
		};
		enum class ELightState
		{
			STATIC,				//Once defined they remain the same for life
			MUTABLE,			//Color and intensity values are changeable but still stand still
			MOBILE				//You can change all values during the runtime but have a high processing cost
		};
		//Criation Window
		class Window
		{
		public:
			Window();
			~Window();
		public:
			// Initialize Window context and open window when ready
			bool init(Random::Structs::SWindow window, Random::Structs::Context context, Random::Structs::Framebuffers framebuffer);
			// Generates a child window using data from the main window
			bool child(Random::Structs::SWindow window);
			// Processes window closure and clears memory data used by the application
			bool release();
			// Process the message loop
			bool broadcast();
			//Returns * ! * true if the window can close
			bool getClose();
			//Skips a rendering when the window is unfocused
			bool getWindowFocus();
			//Render GUI
			bool renderGUI();
			//Return Window Context
			GLFWwindow* getWindow();
			//Return Struct Mouse
			Random::Structs::MouseEvents getMouseEvents();
			const char* glsl_version = "#version 330 core";
		};
		enum EKeyboard
		{
			NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9, NUM_0,
			KEY_1 = GLFW_KEY_1, KEY_2 = GLFW_KEY_2, KEY_3 = GLFW_KEY_3, KEY_4 = GLFW_KEY_4, KEY_5 = GLFW_KEY_5, KEY_6 = GLFW_KEY_6, KEY_7 = GLFW_KEY_7, KEY_8 = GLFW_KEY_8, KEY_9 = GLFW_KEY_9, KEY_0 = GLFW_KEY_0,
			Q = GLFW_KEY_Q, W = GLFW_KEY_W, E = GLFW_KEY_E, R = GLFW_KEY_R, T = GLFW_KEY_T, Y = GLFW_KEY_Y, U = GLFW_KEY_U, I = GLFW_KEY_I, O = GLFW_KEY_O, P = GLFW_KEY_P,
			A = GLFW_KEY_A, S = GLFW_KEY_S, D = GLFW_KEY_D, F = GLFW_KEY_F, G = GLFW_KEY_G, H = GLFW_KEY_H, J = GLFW_KEY_J, K = GLFW_KEY_K, L = GLFW_KEY_L,
			Z = GLFW_KEY_Z, X = GLFW_KEY_X, C = GLFW_KEY_C, V = GLFW_KEY_V, B = GLFW_KEY_B, N = GLFW_KEY_N, M = GLFW_KEY_M,
			NL_ADD = GLFW_KEY_KP_ADD, NL_SUB = GLFW_KEY_KP_SUBTRACT

		};
		enum EMouse
		{
			MOUSE_LEFT = GLFW_MOUSE_BUTTON_LEFT, MOUSE_RIGHT = GLFW_MOUSE_BUTTON_RIGHT, MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
		};
		enum ECursor
		{
			Pointer = GLFW_CURSOR_NORMAL,
			Hide = GLFW_CURSOR_HIDDEN
		};
		class Input : public Window
		{
		public:
			Input();
			~Input();
			bool getKeyPress(EKeyboard key);
			bool getMouseButton(EMouse key);
			void setShowCursor(bool show, ECursor cursor = Pointer);
		};
	}
}

