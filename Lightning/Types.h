#pragma once
//Standard Library
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>

//GLFW
#include <GLFW/glfw3.h>
//GLEW
#include <GLEW/glew.h>
//STB Image importer
#include "stb_image.h"

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace std::chrono;

namespace Lightning
{
	//Typedefs
	
	//unsigned int 32bits
	typedef unsigned int uint;
	//Steady Clock Now
	typedef steady_clock::time_point now;

	//Enums
	enum Keyboard
	{
		NUM_1, NUM_2, NUM_3, NUM_4, NUM_5, NUM_6, NUM_7, NUM_8, NUM_9, NUM_0,
		KEY_1 = GLFW_KEY_1, KEY_2 = GLFW_KEY_2, KEY_3 = GLFW_KEY_3, KEY_4 = GLFW_KEY_4, KEY_5 = GLFW_KEY_5, KEY_6 = GLFW_KEY_6, KEY_7 = GLFW_KEY_7, KEY_8 = GLFW_KEY_8, KEY_9 = GLFW_KEY_9, KEY_0 = GLFW_KEY_0,
		Q = GLFW_KEY_Q, W = GLFW_KEY_W, E = GLFW_KEY_E, R = GLFW_KEY_R, T = GLFW_KEY_T, Y = GLFW_KEY_Y, U = GLFW_KEY_U, I = GLFW_KEY_I, O = GLFW_KEY_O, P = GLFW_KEY_P,
		A = GLFW_KEY_A, S = GLFW_KEY_S, D = GLFW_KEY_D, F = GLFW_KEY_F, G = GLFW_KEY_G, H = GLFW_KEY_H, J = GLFW_KEY_J, K = GLFW_KEY_K, L = GLFW_KEY_L,
		Z = GLFW_KEY_Z, X = GLFW_KEY_X, C = GLFW_KEY_C, V = GLFW_KEY_V, B = GLFW_KEY_B, N = GLFW_KEY_N, M = GLFW_KEY_M,
		NL_ADD = GLFW_KEY_KP_ADD, NL_SUB = GLFW_KEY_KP_SUBTRACT
	};
	enum MouseKeys
	{
		MOUSE_LEFT = GLFW_MOUSE_BUTTON_LEFT, MOUSE_RIGHT = GLFW_MOUSE_BUTTON_RIGHT, MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
	};
	enum Cursor
	{
		Pointer = GLFW_CURSOR_NORMAL,
		Hide = GLFW_CURSOR_HIDDEN
	};
	namespace Flow
	{
		enum Flow
		{
			INPUT, OUTPUT, EXIT, ERROR, EXPECTED, ABORT, PRINT, READ, PROCESS, WARNING
		};
	}
	namespace Colors
	{
		enum ConsoleColor
		{
			BLACK = '\33', RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE,
			BRIGHT_BLACK = 90, BRIGHT_RED, BRIGHT_GREEN, BRIGH_YELLOW, BRIGHT_BLUE,
			BRIGHT_MAGENTA, BRIGHT_CYAN, BRIGHT_WHITE
		};
	}
	
	//Structs
	struct Version
	{
		int Major = 0;
		int Minor = 10;
		int Release = 0;
		int Revision = 0;
		const char* Text = "";
		Version()
		{
			
			this->Major = 0;
			this->Minor = 0;
			this->Release = 0;
			this->Revision = 0;
			
			string mj = to_string(Major);
			string mn = to_string(Minor);
			string rel = to_string(Release);
			string rev = to_string(Revision);
			string text = "(" + mj + '.' + mn + '.' + rel + '.' + rev + ")";
			this->Text = text.c_str();
		}
		Version(int MAJOR, int MINOR, int RELEASE, int REVISION)
		{
			string temp = "";
			this->Major = MAJOR;
			this->Minor = MINOR;
			this->Release = RELEASE;
			this->Revision = REVISION;

			string mj = to_string(Major);
			string mn = to_string(Minor);
			string rel = to_string(Release);
			string rev = to_string(Revision);
			string text = "(" + mj + "." + mn + "." + rel + "." + rev + ")";
			this->Text = text.c_str();
		}
	};

	struct V2
	{
		float x = 0.0f;
		float y = 0.0f;

		V2()
		{
			this->x = 0.0f;
			this->y = 0.0f;
		}
		V2(float new_x, float new_y)
		{
			this->x = new_x;
			this->y = new_y;
		}
		void operator=(const V2& V)
		{
			this->x = V.x;
			this->y = V.y;
		}

	};
	struct V3
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		glm::vec3 glm = glm::vec3(x, y, z);
		V3()
		{
			this->x = 0.0f;
			this->y = 0.0f;
			this->z = 0.0f;
			glm::vec3 glm = glm::vec3(x, y, z);

		}
		V3(float new_x, float new_y, float new_z)
		{
			this->x = new_x;
			this->y = new_y;
			this->z = new_z;
			glm::vec3 glm = glm::vec3(x, y, z);

		}
		V3(float all)
		{
			this->x = all;
			this->y = all;
			this->z = all;
			glm::vec3 glm = glm::vec3(x, y, z);
		}
		void operator=(const V3& V)
		{
			this->x = V.x;
			this->y = V.y;
			this->z = V.z;
			glm::vec3 glm = glm::vec3(x, y, z);
		}
		V3 operator+=(const V3& V)
		{
			V3 temp;
			temp.x = this->x += V.x;
			temp.y = this->y += V.y;
			temp.z = this->z += V.z;
			return temp;
			glm::vec3 glm = glm::vec3(x, y, z);
		}
		V3 operator-=(const V3& V)
		{
			V3 temp;
			temp.x = this->x -= V.x;
			temp.y = this->y -=V.y;
			temp.z = this->z -= V.z;
			return temp;
			glm::vec3 glm = glm::vec3(x, y, z);
		}
		V3 operator*=(const V3& V)
		{
			V3 temp;
			temp.x = this->x *= V.x;
			temp.y = this->y *= V.y;
			temp.z = this->z *= V.z;
			return temp;
			glm::vec3 glm = glm::vec3(x, y, z);
		}
		V3 operator/=(const V3& V)
		{
			V3 temp;
			temp.x = this->x /= V.x;
			temp.y = this->y /= V.y;
			temp.z = this->z /= V.z;
			return temp;
			glm::vec3 glm = glm::vec3(x, y, z);
		}
		V3 operator+(const V3& V)
		{
			V3 temp;
			temp.x = this->x + V.x;
			temp.y = this->y + V.y;
			temp.z = this->z + V.z;
			return temp;
		}
		V3 operator-(const V3& V)
		{
			V3 temp;
			temp.x = this->x - V.x;
			temp.y = this->y - V.y;
			temp.z = this->z - V.z;
			return temp;
		}
		V3 operator*(const V3& V)
		{
			V3 temp;
			temp.x = this->x * V.x;
			temp.y = this->y * V.y;
			temp.z = this->z * V.z;
			return temp;
		}
		V3 operator/(const V3& V)
		{
			V3 temp;
			temp.x = this->x / V.x;
			temp.y = this->y / V.y;
			temp.z = this->z / V.z;
			return temp;
		}
	};
	struct V4
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		float w = 0.0f;

		V4()
		{
			this->x = 0.0f;
			this->y = 0.0f;
			this->z = 0.0f;
			this->w = 0.0f;
		}
		V4(float new_x, float new_y, float new_z, float new_w)
		{
			this->x = new_x;
			this->y = new_y;
			this->z = new_z;
			this->w = new_w;
		}
		void operator=(const V4& V)
		{
			this->x = V.x;
			this->y = V.y;
			this->z = V.z;
			this->w = V.w;
		}
		V4 operator+= (const V4& V)
		{
			this->x += V.x;
			this->y += V.y;
			this->z += V.z;
			this->w += V.w;
		}
		V4 operator-=(const V4& V)
		{
			this->x -= V.x;
			this->y -= V.y;
			this->z -= V.z;
			this->w -= V.w;
		}

	};
	
	struct Transform
	{
		V3 Position;
		V3 Rotation;
		V3 Scale;
		Transform()
		{
			this->Position = V3(0.f);
			this->Rotation = V3(0.f);
			this->Scale = V3(1.f);
		}
		Transform(V3 _position, V3 _rotation, V3 _scale)
		{
			this->Position = _position;
			this->Rotation = _rotation;
			this->Scale = _scale;
		}
		void operator=(const Transform& T)
		{
			this->Position = T.Position;
			this->Rotation = T.Rotation;
			this->Scale = T.Scale;
		}
		Transform operator+(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position + T.Position;
			temp.Rotation = this->Rotation + T.Rotation;
			temp.Scale = this->Scale + T.Scale;
			return temp;
		}
		Transform operator-(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position - T.Position;
			temp.Rotation = this->Rotation - T.Rotation;
			temp.Scale = this->Scale - T.Scale;
			return temp;
		}
		Transform operator*(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position * T.Position;
			temp.Rotation = this->Rotation * T.Rotation;
			temp.Scale = this->Scale * T.Scale;
			return temp;
		}
		Transform operator/(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position / T.Position;
			temp.Rotation = this->Rotation / T.Rotation;
			temp.Scale = this->Scale / T.Scale;
			return temp;
		}
		Transform operator+=(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position += T.Position;
			temp.Rotation = this->Rotation += T.Rotation;
			temp.Scale = this->Scale += T.Scale;
			return temp;
		}
		Transform operator-=(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position -= T.Position;
			temp.Rotation = this->Rotation -= T.Rotation;
			temp.Scale = this->Scale-= T.Scale;
			return temp;
		}
		Transform operator*=(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position *= T.Position;
			temp.Rotation = this->Rotation *= T.Rotation;
			temp.Scale = this->Scale *= T.Scale;
			return temp;
		}
		Transform operator/=(const Transform& T)
		{
			Transform temp;
			temp.Position = this->Position /= T.Position;
			temp.Rotation = this->Rotation /= T.Rotation;
			temp.Scale = this->Scale /= T.Scale;
			return temp;
		}
		void SetPosition(V3 position)
		{
			this->Position = position;
		}
		V3 GetPosition()
		{
			return this->Position;
		}
		void SetRotation(V3 rotation)
		{
			this->Rotation = rotation;
		}
		V3 GetRotation()
		{
			return this->Rotation;
		}
		void SetScale(V3 scale)
		{
			this->Scale = scale;
		}
		V3 GetScale()
		{
			return this->Scale;
		}


};
	
	//Color range 0 - 1
	struct LinearColor
	{
		float r = 0.0f;
		float g = 0.0f;
		float b = 0.0f;
		float a = 0.0f;
		LinearColor()
		{
			this->r = 0.0f;
			this->g = 0.0f;
			this->b = 0.0f;
			this->a = 0.0f;
		}
		LinearColor(float all)
		{
			this->r = all > 1.0f ? 1.0f : all || all < 0.0f ? 0.0f : all;
			this->g = all > 1.0f ? 1.0f : all || all < 0.0f ? 0.0f : all;
			this->b = all > 1.0f ? 1.0f : all || all < 0.0f ? 0.0f : all;
			this->a = all > 1.0f ? 1.0f : all || all < 0.0f ? 0.0f : all;
		}
		LinearColor(float _r, float _g, float _b, float _a)
		{
			this->r = _r > 1.0f ? 1.0f : _r || _r < 0.0f ? 0.0f : _r;
			this->g = _g > 1.0f ? 1.0f : _g || _g < 0.0f ? 0.0f : _g;
			this->b = _b > 1.0f ? 1.0f : _b || _b < 0.0f ? 0.0f : _b;
			this->a = _a > 1.0f ? 1.0f : _a || _a < 0.0f ? 0.0f : _a;
		}
		void operator=(const LinearColor& l)
		{
			this->r = l.r;
			this->g = l.g;
			this->b = l.b;
			this->a = l.a;
		}
		LinearColor operator+(const LinearColor& l)
		{
			LinearColor temp;
			temp.r = this->r + l.r;
			temp.g = this->g + l.g;
			temp.b = this->b + l.b;
			temp.a = this->a + l.a;
			return temp;
		}
		LinearColor operator-(const LinearColor& l)
		{
			LinearColor temp;
			temp.r = this->r - l.r;
			temp.g = this->g - l.g;
			temp.b = this->b - l.b;
			temp.a = this->a - l.a;
			return temp;
		}
		LinearColor operator*(const LinearColor& l)
		{
			LinearColor temp;
			temp.r = this->r * l.r;
			temp.g = this->g * l.g;
			temp.b = this->b * l.b;
			temp.a = this->a * l.a;
			return temp;
		}
		LinearColor operator/(const LinearColor& l)
		{
			LinearColor temp;
			temp.r = this->r / l.r;
			temp.g = this->g / l.g;
			temp.b = this->b / l.b;
			temp.a = this->a / l.a;
			return temp;
		}

	};

	struct name
	{
	private:
		const char* s = "";
	public:
		name()
		{
			s = "";
		}
		name(const char* n)
		{
			this->s = n;
		}
		void operator =(const name& n)
		{
			this->s = n.s;
		}
		void operator =(const char* n)
		{
			this->s = n;
		}
		bool operator ==(const name& n)
		{
			int l = (int)std::strlen(s);
			int ls = (int)std::strlen(n.s);
			int p = 0;

			if (l == ls)
			{
				for (int i = 0; i < l; i++)
				{
					if (s[i] == n.s[i])
						p++;
					else
						p--;
				}
			}
			else
				return false;

			return (p == l ? true : false);

		}
		name operator +=(const name& n)
		{

		}
		name operator -=(const name& n)
		{

		}
	};
	

	//Functions

	float IncrementRangef(char signal, float target, float value, float min = 0.f, float max = 1.f);

}