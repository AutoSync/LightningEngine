#pragma once
//Standard Library
#include <cstdio>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <sstream>
#include <iostream>

//GLEW
#include <GLEW/glew.h>
//GLFW
#include <GLFW/glfw3.h>
//STB Image importer
#include "stb_image.h"

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//MACROS

//MATH  
#define LIGHTNING_PI 3.141592653589
#define LIGHTNING_RADIANS 0.01745329251994329576923690768489
#define LIGHTNING_DEGREES 57.21684788166867


// Lightning Bool
#define LFALSE			 0x0000		// Lightning Bool False
#define LTRUE			 0x0001		// Lightning Bool True
#define LDEFAULT		 0x0002		// Lightning Default Command

//Lightning Rendering
//
#define LR_DEPTH		0x0003	//Flag * Rendering Depth
#define DEPTH_ALWAYS	4		//Function fn The depth test always passes.
#define DEPTH_NEVER		5		//Function fn The depth test never passes.
#define DEPTH_LESS		6		//Function fn Passes if depth < Buffer stored
#define DEPTH_EQUAL		7		//Function fn Passes if depth == Buffer stored
#define DEPTH_LEQUAL	8		//
#define DEPTH_GREATER	9
#define DEPTH_NOTEQUAL	10 
#define DEPTH_GEQUAL	11
#define DEPTH_NOMASK	12		//Condition * disable mask depth
#define LR_STENCIL		200
#define LR_BLEND		201
#define LR_CULL			202
#define LR_CULL_BACK	203
#define LR_CULL_CCW		204
#define LR_				205
#define LR_CLEAR		206

//usings

using namespace std;
using namespace std::chrono;

namespace Lightning
{
	//Typedefs
	
	
	typedef unsigned int uint;					//Unsigned int 
	typedef unsigned char uchar;				//Unsigned char
	//Steady Clock Now
	typedef steady_clock::time_point now;

	//Enums
	
	//Selectable Light type
	enum LightType
	{
		Direct, Spotlight, Point, Rect
	};
	//Mobility objects
	enum Mobility
	{
		Static, Mutable, Mobile
	};

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
		MOUSE_LEFT = GLFW_MOUSE_BUTTON_1, MOUSE_RIGHT = GLFW_MOUSE_BUTTON_2, MOUSE_MIDDLE = GLFW_MOUSE_BUTTON_MIDDLE
	};
	enum Cursor
	{
		Pointer = GLFW_CURSOR_NORMAL,
		Hide = GLFW_CURSOR_HIDDEN
	};
	//Enum Vector Direction
	enum Direction
	{
		FORWARD, BACKWARD, RIGHT, LEFT, UP, DOWN
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

	//VERSION
	struct Version
	{
		int Major = 0;
		int Minor = 10;
		int Release = 0;
		int Revision = 0;
		const char* Text = " ";
		Version()
		{		
			this->Major = 0;
			this->Minor = 0;
			this->Release = 0;
			this->Revision = 0;
			this->Text = "";
		}
		Version(int MAJOR, int MINOR, int RELEASE, int REVISION)
		{
			this->Major = MAJOR;
			this->Minor = MINOR;
			this->Release = RELEASE;
			this->Revision = REVISION;
			this->Text = "";
		}
		Version(int MAJOR, int MINOR, int RELEASE, int REVISION, const char* TEXT)
		{
			this->Major = MAJOR;
			this->Minor = MINOR;
			this->Release = RELEASE;
			this->Revision = REVISION;
			this->Text = TEXT;
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
		void operator=(const glm::vec3& V)
		{
			this->x = V.x;
			this->y = V.y;
			this->z = V.z;
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
		glm::vec3 GetGLM()
		{
			return glm::vec3(x, y, z);
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
	struct D2
	{
		double x = 0.f, y = 0.f;
		D2()
		{
			this->x = 0.f;
			this->y = 0.f;
		}
		D2(double SCALAR)
		{
			this->x = SCALAR;
			this->y = SCALAR;
		}
		D2(double X, double Y)
		{
			this->x = X;
			this->y = Y;
		}
		void operator=(const D2& v)
		{
			this->x = v.x;
			this->y = v.y;
		}
		D2 operator+(const D2& v)
		{
			D2 t;
			t.x = this->x + v.x;
			t.y = this->y + v.y;
			return t;
		}
		D2 operator-(const D2& v)
		{
			D2 t;
			t.x = this->x - v.x;
			t.y = this->y - v.y;
			return t;
		}
		D2 operator*(const D2& v)
		{
			D2 t;
			t.x = this->x * v.x;
			t.y = this->y * v.y;
			return t;
		}
		D2 operator/(const D2& v)
		{
			D2 t;
			t.x = this->x / v.x;
			t.y = this->y / v.y;
			return t;
		}
		D2 operator+=(const D2& v)
		{
			D2 t;
			t.x = this->x += v.x;
			t.y = this->y += v.y;
			return t;
		}
		D2 operator-=(const D2& v)
		{
			D2 t;
			t.x = this->x -= v.x;
			t.y = this->y -= v.y;
			return t;
		}
		D2 operator*=(const D2& v)
		{
			D2 t;
			t.x = this->x *= v.x;
			t.y = this->y *= v.y;
			return t;
		}
		D2 operator/=(const D2& v)
		{
			D2 t;
			t.x = this->x /= v.x;
			t.y = this->y /= v.y;
			return t;
		}
	};
	struct D3
	{
		double x = 0.f, y = 0.f, z = 0.f;
		D3()
		{
			this->x = 0.f;
			this->y = 0.f;
			this->z = 0.f;
		}
		D3(double SCALAR)
		{
			this->x = SCALAR;
			this->y = SCALAR;
			this->z = SCALAR;
		}
		D3(double X, double Y, double Z)
		{
			this->x = X;
			this->y = Y;
			this->z = Z;
		}
		void operator=(const D3& v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
		}
		D3 operator+(const D3& v)
		{
			D3 t;
			t.x = this->x + v.x;
			t.y = this->y + v.y;
			t.z = this->z + v.z;
			return t;
		}
		D3 operator-(const D3& v)
		{
			D3 t;
			t.x = this->x - v.x;
			t.y = this->y - v.y;
			t.z = this->z - v.z;
			return t;
		}
		D3 operator*(const D3& v)
		{
			D3 t;
			t.x = this->x * v.x;
			t.y = this->y * v.y;
			t.z = this->z * v.z;
			return t;
		}
		D3 operator/(const D3& v)
		{
			D3 t;
			t.x = this->x / v.x;
			t.y = this->y / v.y;
			t.z = this->z / v.z;
			return t;
		}
		D3 operator+=(const D3& v)
		{
			D3 t;
			t.x = this->x += v.x;
			t.y = this->y += v.y;
			t.z = this->z += v.z;
			return t;
		}
		D3 operator-=(const D3& v)
		{
			D3 t;
			t.x = this->x -= v.x;
			t.y = this->y -= v.y;
			t.z = this->z -= v.z;
			return t;
		}
		D3 operator*=(const D3& v)
		{
			D3 t;
			t.x = this->x *= v.x;
			t.y = this->y *= v.y;
			t.z = this->z *= v.z;
			return t;
		}
		D3 operator/=(const D3& v)
		{
			D3 t;
			t.x = this->x /= v.x;
			t.y = this->y /= v.y;
			t.z = this->z /= v.z;
			return t;
		}
	};
	struct D4
	{
		double x = 0.f, y = 0.f, z = 0.f, w = 0.f;
		D4()
		{
			this->x = 0.f;
			this->y = 0.f;
			this->z = 0.f;
			this->w = 0.f;
		}
		D4(double SCALAR)
		{
			this->x = SCALAR;
			this->y = SCALAR;
			this->z = SCALAR;
			this->w = SCALAR;
		}
		D4(double X, double Y, double Z, double W)
		{
			this->x = X;
			this->y = Y;
			this->z = Z;
			this->w = W;
		}
		void operator=(const D4& v)
		{
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
			this->w = v.w;
		}
		D4 operator+(const D4& v)
		{
			D4 t;
			t.x = this->x + v.x;
			t.y = this->y + v.y;
			t.z = this->z + v.z;
			t.w = this->w + v.w;
			return t;
		}
		D4 operator-(const D4& v)
		{
			D4 t;
			t.x = this->x - v.x;
			t.y = this->y - v.y;
			t.z = this->z - v.z;
			t.w = this->w - v.w;
			return t;
		}
		D4 operator*(const D4& v)
		{
			D4 t;
			t.x = this->x * v.x;
			t.y = this->y * v.y;
			t.z = this->z * v.z;
			t.w = this->w * v.w;
			return t;
		}
		D4 operator/(const D4& v)
		{
			D4 t;
			t.x = this->x / v.x;
			t.y = this->y / v.y;
			t.z = this->z / v.z;
			t.w = this->w / v.w;
			return t;
		}
		D4 operator+=(const D4& v)
		{
			D4 t;
			t.x = this->x += v.x;
			t.y = this->y += v.y;
			t.z = this->z += v.z;
			t.w = this->w += v.w;
			return t;
		}
		D4 operator-=(const D4& v)
		{
			D4 t;
			t.x = this->x -= v.x;
			t.y = this->y -= v.y;
			t.z = this->z -= v.z;
			t.w = this->w -= v.w;
			return t;
		}
		D4 operator*=(const D4& v)
		{
			D4 t;
			t.x = this->x *= v.x;
			t.y = this->y *= v.y;
			t.z = this->z *= v.z;
			t.w = this->w *= v.w;
			return t;
		}
		D4 operator/=(const D4& v)
		{
			D4 t;
			t.x = this->x /= v.x;
			t.y = this->y /= v.y;
			t.z = this->z /= v.z;
			t.w = this->w /= v.w;
			return t;
		}
	};
	struct Vertex
	{
		V3 Position;		// Position Vertex
		V2 TexCoords;		// Texture Cordinates
		V3 Normal;			// Normal Vertex 
		V3 Tangent;			// Tangents Vertex 
		V3 Bitangent;		// Bitangens Vertex
	};
	struct Texture
	{
		uint	 Id;
		string type;
		string path;
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
	
	//Color range V4 0 - 1
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
		LinearColor(float SCALAR)
		{
			this->r = SCALAR;
			this->g = SCALAR;
			this->b = SCALAR;
			this->a = SCALAR;
		}
		LinearColor(float _r, float _g, float _b, float _a)
		{
			this->r = _r;
			this->g = _g;
			this->b = _b;
			this->a = _a;
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
	struct C3
	{
		float r = 0;
		float g = 0;
		float b = 0;
		C3()
		{
			this->r = 0;
			this->g = 0;
			this->b = 0;
		}
		C3(int SCALAR)
		{
			this->r = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
			this->g = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
			this->b = (SCALAR <= 0) ? 0 : ((float)SCALAR / 255);
		}
		C3(int RED, int GREEN, int BLUE)
		{
			this->r = (RED <= 0) ? 0 : ((float)RED / 255);
			this->g = (GREEN <= 0) ? 0 : ((float)GREEN / 255);
			this->b = (BLUE <= 0) ? 0 : ((float)BLUE / 255);
		}
		void operator=(const C3& c)
		{
			this->r = c.r;
			this->g = c.g;
			this->b = c.b;
		}
	};
	struct C4
	{
		int r = 0;
		int g = 0;
		int b = 0;
		int a = 0;
		C4()
		{
			this->r = 0;
			this->g = 0;
			this->b = 0;
			this->a = 0;

		}
		C4(int all)
		{
			this->r = all > 255 ? 255 : all || all < 0 ? 0 : all;
			this->g = all > 255 ? 255 : all || all < 0 ? 0 : all;
			this->b = all > 255 ? 255 : all || all < 0 ? 0 : all;
			this->a = all > 255 ? 255 : all || all < 0 ? 0 : all;

		}
		C4(int Red, int Green, int Blue, int Alpha)
		{
			this->r = Red > 255 ? 255 : Red || Red < 0 ? 0 : Red;
			this->g = Green > 255 ? 255 : Green || Green < 0 ? 0 : Green;
			this->b = Blue > 255 ? 255 : Blue || Blue < 0 ? 0 : Blue;
			this->a = Alpha > 255 ? 255 : Alpha || Alpha < 0 ? 0 : Alpha;

		}
		void operator=(const C4& c)
		{
			this->r = c.r;
			this->g = c.g;
			this->b = c.b;
			this->a = c.a;
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
	
	//ENGINE STRUCTS
	//Has a set of definitions for rendering OpenGL
	struct RenderSettings
	{
		bool Recolor = false;
		V4 color = V4();
	};
	//Initial setup to define window creation
	struct EngineSettings
	{
		int width = 840;
		int height = 480;
		const char* title = "Lightning Engine";
		Version version = Version(0,10,0,0);
		int version_major = 4;
		int version_minor = 5;
		bool vsync = false;
		bool core_profile = true;
		bool doubleFrame = false;
		bool framerate = true;
		bool displayVersion = true;
		bool AA = false;
		bool cansize = true;
		EngineSettings()
		{
			width = 840;
			height = 480;
			title = "Lightning Engine";
			version = Version(0, 10, 0, 0);
			version_major = 4;
			version_minor = 5;
			vsync = false;
			core_profile = true;
			doubleFrame = false;
			framerate = true;
			displayVersion = true;
			AA = false;
			cansize = true;
		}
		EngineSettings(int W, int H, const char* TITLE, bool CANSIZE)
		{
			width = W;
			height = H;
			title = TITLE;
			version = Version(0, 10, 0, 0);
			version_major = 4;
			version_minor = 5;
			vsync = false;
			core_profile = true;
			doubleFrame = false;
			framerate = true;
			displayVersion = true;
			AA = false;
			cansize = CANSIZE;
		}
	};
	struct MouseEvents
	{
		D2 position = D2(0.0, 0.0);			//Mouse position relative to your window
		D2 scroll = D2(0.0, 0.0);				//Scroll scroll relative to your window
		bool scrollUp = false, scrollDown = false;
		MouseEvents()
		{
			position = D2(0.0);
			scroll = D2(0.0);
			scrollUp = false;
			scrollDown = false;
		}
		MouseEvents(D2 MOUSE, D2 SCROLL)
		{
			this->position = MOUSE;
			this->scroll = SCROLL;
		}
		MouseEvents(double mx, double my, double sx, double sy)
		{
			this->position.x = mx;
			this->position.y = my;
			this->scroll.x = sx;
			this->scroll.y = sy;
		}
		MouseEvents(const MouseEvents& m)
		{
			position = m.position;
			scroll = m.scroll;
		}
		void operator = (const MouseEvents& m)
		{
			position = m.position;
			scroll = m.scroll;
		}
		bool GetScrollUp()
		{
			return scrollUp;
		}
		bool GetScrollDown()
		{
			return scrollDown;
		}
	};

	//Functions

	//Increment into range float
	float IncrementRangef(char signal, float target, float value, float min = 0.f, float max = 1.f)
	{
		if (target > max)
			target = max;
		else if (target < min)
			target = min;
		else
		{
			switch (signal)
			{
			case '+':
				target += value;
				break;
			case '-':
				target -= value;
				break;
			}
		}
		return target;
	}
	//Rotate by vector 3
	glm::vec3 MakeRotate(glm::vec3 rotation)
	{
		glm::vec3 vector;
		vector.x = cos(radians(rotation.x) * cos(radians(rotation.y)));
		vector.y = sin(radians(rotation.y));
		vector.z = sin(radians(rotation.x) * cos(radians(rotation.y)));
		return glm::normalize(vector);
	}
	//Convert Degress to Radians
	float radians(float degrees)
	{
		return degrees * LIGHTNING_RADIANS;
	}
	//Convert Radians to Degress
	float degrees(float radians)
	{
		return radians * LIGHTNING_DEGREES;
	}

	//Classes

	//Class to clamp values 
	class SClamp
	{
	private:
		int imin = 0, imax = 1;
		float fmin = 0.0, fmax = 1.0f;
		double dmin = 0.0, dmax = 1.0f;
	public:
		//Constructor empty
		SClamp()
		{
			imin = 0;
			imax = 1;
			fmin = 0.0f;
			fmax = 1.0f;
			dmin = 0.0;
			dmax = 1.0;
		}
		//Constructor sets min and max values
		SClamp(int MIN, int MAX)
		{
			this->imin = MIN;
			this->imax = MAX;
		}
		//Constructor sets min and max values
		SClamp(float MIN, float MAX)
		{
			this->fmin = MIN;
			this->fmax = MAX;
		}
		//Constructor sets min and max values
		SClamp(double MIN, double MAX)
		{
			this->dmin = MIN;
			this->dmax = MAX;
		}
		//Constructor Copy
		SClamp(const SClamp& clamp)
		{
			fmin = clamp.fmin;
			fmax = clamp.fmax;
			dmin = clamp.dmin;
			dmax = clamp.dmax;
		}
		//Initializator void 
		void operator = (const SClamp& clamp)
		{
			fmin = clamp.fmin;
			fmax = clamp.fmax;
			dmin = clamp.dmin;
			dmax = clamp.dmax;
		}
		//Clamp int input
		int clamp(int INPUT)
		{
			if (INPUT < imin)
				return imin;
			if (INPUT > imax)
				return imax;
			return INPUT;
		}
		//Clamp input with min and max values
		int clamp(int INPUT, int MIN, int MAX)
		{
			this->imin = MIN;
			this->imax = MAX;
			if (INPUT < imin)
				return imin;
			if (INPUT > imax)
				return imax;
			return INPUT;
		}
		//Clamp float input
		float clamp(float INPUT)
		{
			if (INPUT < fmin)
				return fmin;
			if (INPUT > fmax)
				return fmax;
			return INPUT;
		}
		//Clamp input with min and max values
		float clamp(float INPUT, float MIN, float MAX)
		{
			this->fmin = MIN;
			this->fmax = MAX;
			if (INPUT < fmin)
				return fmin;
			if (INPUT > fmax)
				return fmax;
			return INPUT;
		}
		//Clamp double input
		double clamp(double INPUT)
		{
			if (INPUT < dmin)
				return dmin;
			if (INPUT > dmax)
				return dmax;
			return INPUT;
		}
		//clamp input with min and max values
		double clamp(double INPUT, double MIN, double MAX)
		{
			this->dmin = MIN;
			this->dmax = MAX;
			if (INPUT < dmin)
				return dmin;
			if (INPUT > dmax)
				return dmax;
			return INPUT;
		}
	};
}