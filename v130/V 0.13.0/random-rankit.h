#pragma once
#ifndef RANDOM_RANKIT
#define RANDOM_RANKIT

#include <random>
#include <iostream>
#include <vector>
#include <string>

//Variables Typedefs

typedef unsigned int uint;		//Lightning Engine Type from unsigned int
typedef int flag;				//Lightning Engine Type from int
typedef std::string text;		//Lightning Engine Type from string
typedef const char* solidtext;	//Lightning Engine Type from const char*
//Macros
#define RFALSE					0		// Set Random Rankit False
#define RTRUE					1		// Set Random Rankit True
#define RI_GL					2		// Random Input type OpenGL GLSL
#define RI_DX					3		// Random Input type DirectX HLSL
#define RI_SHADER				4		// input flag for Equinox Shader object
#define RI_IMAGE 				5		// Input flag of a image
#define RI_TEXTURE				6		// Input flag of a Texture
#define RI_MODEL				7		// Input flag for a 3d model
#define RI_AUDIO				8		// Input flag for an audio track
#define RI_CODE					9		// Input flag for a source code
#define RI_LEVEL				10		// Input flag for Level Object
#define RI_HURRICANE			11		// Input flag for Hurricane Particles
#define RI_MOTIONPHYSIX			12		// Input flag for MotionPhysix Objects
#define RI_CUBEMAP				13		// Input flag for Texture cubemap
#define RI_MOVIE				14		// Input flag for Movie or Clip of Video
#define RI_TITANFLOW			15		// Input flag for Titan Flow Object
#define RI_VDFLOW				16		// Input flag for Vision Direct Flow Object
#define RI_ATLAS				17		// Input flag for Atlas flow Object
#define RI_CONTINUOUSFLOW		18		// Input flag for Continuous Flow Object Master
#define RO_SHADER				19		// Output flag for Equinox shader object
#define RO_IMAGE				20		// Output flag for of a image
#define RO_TEXTURE				21		// Output flag for of a texture
#define RO_MODEL				22		// Output flag for a 3d model
#define RO_AUDIO				23		// Output flag for an audio track
#define RO_CODE					24		// Output flag for a source code
#define RO_LEVEL				25		// Output flag for Level Object
#define RO_HURRICANE			26		// Output flag for Hurricane Particles
#define RO_MOTIONPHYSIX			27		// Output flag for MotionPhysix Objects
#define RO_CUBEMAP				28		// Output flag for Texture cubemap
#define RO_MOVIE				29		// Output flag for Movie or Clip of Video
#define RO_TITANFLOW			30		// Output flag for Titan Flow Object
#define RO_VDFLOW				31		// Output flag for Vision Direct Flow Object
#define RO_ATLAS				32		// Output flag for Atlas flow Object
#define RO_CONTINUOUSFLOW		33		// Output flag for Continuous Flow Object Master
#define RI_LIGHTNING_BYTECODE	34		//Input flag for Lightning Engine Bytecode source code
#define RO_LIGHTNING_BYTECODE	35		//Output flag for Lightning Engine Bytecode source code
//MATH
#define RDN_PI 3.141592653589
#define RDN_RADIANS 0.01745329251994329576923690768489
#define RDN_DEGREES 57.21684788166867

namespace Random
{
	namespace Enums
	{
		// List of types of textures for internal treatments
		enum EImageType
		{
			IT_ICON,
			IT_IMAGE,
			IT_SPLASH,
			IT_BANNER,
			IT_VECTOR,
			IT_TITAN,
			IT_SPRITE,
			IT_TILESET,
			IT_COLORLUT
		};
		//List of mouse states
		enum class EMouseStates
		{
			M_HOVER,
			M_LEFTCLICK,
			M_DRAG,
			M_DROP,
			M_MID
		};
		// Vector Movement
		enum class EVectorMovement
		{
			FORWARD = 0,
			BACKWARD = 1,
			LEFT = 2,
			RIGHT = 3,
			UP = 4,
			DOWN = 5
		};
	}

	namespace Structs
	{
		//Typedefs Structs

		//Logic is a structure with members of the Boolean type that can be used to nest conditions
		struct logic
		{
		private:
			bool Default = true;
		public:
			//The boolean of this structure
			bool Value = false;
			//Default Constructor
			logic();
			// Sets the Initial value and the default value
			//!The default value cannot be changed after initialization
			logic(bool _value, bool _default);
			// Constructor sets only the value
			logic(bool b);
			void operator=(const logic& l);
			//Returns the OU of the structure with operator
			bool OR(bool _operator);
			//Returns the XOR of the structure with operator
			bool XOR(bool _operator);
			//Returns the AND of the structure with operator
			bool AND(bool _operator);
			//Returns the NAND of the structure with operator
			bool NAND(bool _operator);
			//Returns the NOT of the structure
			bool NOT();
			// Reset the value with the default boolean
			void RevertToDefault();
			//When the condition is equal to the "Value" of the variable, the object
			//calls a void return function with no arguments
			void WHEN(bool _condition, void (*_function)());
		};
		//Clamp structure to fix data data with minimum and maximum value
		class SClamp
		{
		private:
			int imin = 0, imax = 1;
			float fmin = 0.0, fmax = 1.0f;
			double dmin = 0.0, dmax = 1.0f;
		public:
			//Constructor empty
			SClamp();
			//Constructor sets min and max values
			SClamp(int MIN, int MAX);
			//Constructor sets min and max values
			SClamp(float MIN, float MAX);
			//Constructor sets min and max values
			SClamp(double MIN, double MAX);
			//Constructor Copy
			SClamp(const SClamp& clamp);
			//Initializator void 
			void operator = (const SClamp& clamp);
			//Clamp int input
			int clamp(int INPUT);
			//Clamp input with min and max values
			int clamp(int INPUT, int MIN, int MAX);
			//Clamp float input
			float clamp(float INPUT);
			//Clamp input with min and max values
			float clamp(float INPUT, float MIN, float MAX);
			//Clamp double input
			double clamp(double INPUT);
			//clamp input with min and max values
			double clamp(double INPUT, double MIN, double MAX);
		};
		// Vector 2 Positions
		struct v2
		{
			float x, y;
			v2();
			v2(float X, float Y);
			v2(float SCALAR);
			v2(const v2& vector);
			void operator= (const v2& vector);
			v2 operator+= (const v2& vector);
			v2 operator-= (const v2& vector);
			v2 operator*= (const v2& vector);
			v2 operator/=(const v2& vector);
			v2 operator+ (const v2& vector);
			v2 operator- (const v2& vector);
			v2 operator* (const v2& vector);
			v2 operator/ (const v2& vector);
		};
		// Vector 3 Positions
		struct v3
		{
			float x, y, z;
			v3();
			v3(float X, float Y, float Z);
			v3(float SCALAR);
			v3(const v3& vector);
			v3 getForwardVector();
			v3 getRightVector();
			v3 getUpVector();
			void operator= (const v3& vector);
			v3 operator+=(const v3& vector);
			v3 operator-=(const v3& vector);
			v3 operator*=(const v3& vector);
			v3 operator/=(const v3& vector);
			v3 operator+ (const v3& vector);
			v3 operator- (const v3& vector);
			v3 operator* (const v3& vector);
			v3 operator/ (const v3& vector);
		};
		// Vector 4 Positions
		struct v4
		{
			float x = 0, y = 0, z = 0, w = 0;
			v4();
			v4(float X, float Y, float Z, float W);
			v4(float SCALAR);
			v4(const v4& vector);
			void operator = (const v4& vector);
			v4 operator+= (const v4& vector);
			v4 operator-= (const v4& vector);
			v4 operator*= (const v4& vector);
			v4 operator/= (const v4& vector);
			v4 operator+ (const v4& vector);
			v4 operator- (const v4& vector);
			v4 operator* (const v4& vector);
			v4 operator/ (const v4& vector);
		};
		// Vector 2 INT Positions
		struct iv2
		{
			int x = 0, y = 0;
			iv2();
			iv2(int X, int Y);
			iv2(int SCALAR);
			iv2(const iv2& vector);
			void operator= (const iv2& vector);
			iv2 operator+= (const iv2& vector);
			iv2 operator-= (const iv2& vector);
			iv2 operator*= (const iv2& vector);
			iv2 operator/= (const iv2& vector);
			iv2 operator+ (const iv2& vector);
			iv2 operator- (const iv2& vector);
			iv2 operator* (const iv2& vector);
			iv2 operator/ (const iv2& vector);
		};
		// Vector 3 INT Positions
		struct iv3
		{
			int x = 0, y = 0, z = 0;
			iv3();
			iv3(int X, int Y, int Z);
			iv3(int SCALAR);
			iv3(const iv3& vector);
			void operator = (const iv3& vector);
			iv3 operator+= (const iv3& vector);
			iv3 operator-= (const iv3& vector);
			iv3 operator*= (const iv3& vector);
			iv3 operator/= (const iv3& vector);
			iv3 operator+ (const iv3& vector);
			iv3 operator- (const iv3& vector);
			iv3 operator* (const iv3& vector);
			iv3 operator/ (const iv3& vector);
		};
		// Vector 4 INT Positions
		struct iv4
		{
			int x = 0, y = 0, z = 0, w = 0;
			iv4();
			iv4(int X, int Y, int Z, int W);
			iv4(int SCALAR);
			iv4(const iv4& vector);
			void operator = (const iv4& vector);
			iv4 operator+= (const iv4& vector);
			iv4 operator-= (const iv4& vector);
			iv4 operator*= (const iv4& vector);
			iv4 operator/= (const iv4& vector);
			iv4 operator+ (const iv4& vector);
			iv4 operator- (const iv4& vector);
			iv4 operator* (const iv4& vector);
			iv4 operator/ (const iv4& vector);
		};
		// Vector 2 Double Positions
		struct dv2
		{
			double x = 0, y = 0;
			dv2();
			dv2(double X, double Y);
			dv2(double SCALAR);
			dv2(const dv2& vector);
			void operator = (const dv2& vector);
			dv2 operator+= (const dv2& vector);
			dv2 operator-= (const dv2& vector);
			dv2 operator*= (const dv2& vector);
			dv2 operator/= (const dv2& vector);
			dv2 operator+ (const dv2& vector);
			dv2 operator- (const dv2& vector);
			dv2 operator* (const dv2& vector);
			dv2 operator/ (const dv2& vector);
		};
		// Vector 3 Double Positions
		struct dv3
		{
			double x = 0, y = 0, z = 0;
			dv3();
			dv3(double X, double Y, double Z);
			dv3(double SCALAR);
			dv3(const dv3& vector);
			void operator = (const dv3& vector);
			dv3 operator+= (const dv3& vector);
			dv3 operator-= (const dv3& vector);
			dv3 operator*= (const dv3& vector);
			dv3 operator/= (const dv3& vector);
			dv3 operator+ (const dv3& vector);
			dv3 operator- (const dv3& vector);
			dv3 operator* (const dv3& vector);
			dv3 operator/ (const dv3& vector);
		};
		// Vector 4 Double positions
		struct dv4
		{
			double x = 0, y = 0, z = 0, w = 0;
			dv4();
			dv4(double X, double Y, double Z, double W);
			dv4(double SCALAR);
			dv4(const dv4& vector);
			void operator = (const dv4& vector);
			dv4 operator+= (const dv4& vector);
			dv4 operator-= (const dv4& vector);
			dv4 operator*= (const dv4& vector);
			dv4 operator/= (const dv4& vector);
			dv4 operator+ (const dv4& vector);
			dv4 operator- (const dv4& vector);
			dv4 operator* (const dv4& vector);
			dv4 operator/ (const dv4& vector);
		};
		// 2D axis rotation
		struct r2
		{
		private:
			float x = 0.0f, y = 0.0f;
			bool ConstrainXaxis = false, ConstrainYaxis = false;
			SClamp clampX, clampY;
		public:
			r2();
			r2(float X, float Y);
			r2(float SCALAR);
			r2(float X, bool isConstraintX, SClamp ConstraintX, 
				float Y, bool isConstraintY, SClamp ConstrainY);
			void setConstraints(SClamp CONSTRAIN_X, SClamp CONSTRAIN_Y);
			void setConstrainX(SClamp X);
			void setConstrainY(SClamp Y);
			r2(const r2& rt);
			void operator= (const r2& rt);
		};
		// 3D axis rotation
		struct r3
		{
			float x = 0.0f, y = 0.0f, z = 0.0f;
			bool ConstrainXaxis = false, ConstrainYaxis = false, ConstrainZaxis = false;
			SClamp clampX, clampY, clampZ;
			r3();
			r3(float X, float Y, float Z);
			r3(float SCALAR);
			r3(float X, bool isConstraintX, SClamp ConstrainX, 
				float Y, bool isConstraintY, SClamp ConstrainY, 
				float Z, bool isConstraintZ, SClamp ConstraintZ);
			void setConstraints(SClamp CONSTRAIN_X, SClamp CONTRAIN_Y, SClamp CONSTRAIN_Z);
			void setConstrainX(SClamp X);
			void setConstrainY(SClamp Y);
			void setConstrainZ(SClamp Z);
			r3(const r3& rt);
			void operator = (const r3& rt);
		};
		// Receives an Integer of 0-255 and stores a 0.0-1.0f float
		struct Transform
		{
			v3 Location = v3(0.0f);
			r3 Rotation = r3(0.0f);
			v3 Scale = v3(1.0f);
			Transform();
			Transform(v3 NewLocation, r3 NewRotation, v3 NewScale);
			Transform(const Transform& T);
			void operator=(const Transform& T);
		};
		struct c3
		{
			float r = 0, g = 0, b = 0;
		public:
			c3();
			c3(int RED, int GREEN, int BLUE);
			c3(int SCALAR);
			c3(const c3& color);
			void operator= (const c3& color);
			c3 operator * (const c3& color);
		};
		// Receives an Integer of 0-255 and stores a 0.0-1.0f float
		struct c4
		{
			float r = 0, g = 0, b = 0, a = 0;
		public:
			c4();
			c4(int RED, int GREEN, int BLUE, int ALPHA);
			c4(int SCALAR);
			c4(const c4& color);
			void operator= (const c4& color);
		};
		// Struct for 3D objects
		struct Vertex
		{
			v3 Location;		// Location Vertex
			v2 TexCoords;		// Texture Cordinates
			v3 Normal;			// Normal Vertex 
			v3 Tangent;			// Tangents Vertex 
			v3 Bitangent;		// Bitangens Vertex
		};
		// Struct to Texture
		struct Texture
		{
			uint ID;				// Binary from Texture
			text type;			// 
			text path;
		};
		// Handler for icons
		struct HIconHandler
		{
			int width;
			int height;
			unsigned char* pixels;
			HIconHandler();
			HIconHandler(const HIconHandler& h);
			void operator = (const HIconHandler& h);
		};
		// Window Settings 
		struct SWindow
		{
			int width, height;
			text title;
			bool canSize;
			SWindow();
			SWindow(int WIDTH, int HEIGHT, text TITLE, bool CANSIZE);
			SWindow(const SWindow& ws);
			void operator=(const SWindow& ws);
		};
		// Context Creation
		struct Context
		{
			//Use The latest API Version
			int major_version = 3;
			//Set the minimum supported API version
			int minor_version = 3;
			//Use only current API version
			bool core = true;
			//use only deprecated API version
			bool minimal_core = false;
			Context();
			Context(int MAJOR_VERSION, int MINOR_VERSION, bool CORE);
			Context(int MAJOR_VERSION, int MINOR_VERSION, bool CORE, bool MINIMAL_CORE);
			Context(const Context& cs);
			void operator=(const Context& cs);
			
		};
		// Framebuffer Settings
		struct Framebuffers
		{
			unsigned int flags;
			bool msaa = false;
			bool vsync = false;
			Framebuffers();
			Framebuffers(int FLAG, bool MSAA, bool VSYNC);
			Framebuffers(const Framebuffers& fb);
			void operator=(const Framebuffers& fb);
		};
		// Events Mouses Handler
		struct MouseEvents
		{
			dv2 position = dv2(0.0, 0.0);			//Mouse position relative to your window
			dv2 scroll = dv2(0.0, 0.0);				//Scroll scroll relative to your window
			bool scrollUp = false, scrollDown = false;
			MouseEvents();
			MouseEvents(dv2 MOUSE, dv2 SCROLL);
			MouseEvents(double mx, double my, double sx, double sy);
			MouseEvents(const MouseEvents& m);
			void operator = (const MouseEvents& m);
			bool getScrollUp();
			bool getScrollDown();
		};
		struct Viewport
		{
			float nearclip = 0.1f;
			float farclip = 100.0f;
			int view_pos_x = 0;
			int view_pos_y = 0;
			Viewport();
			Viewport(float new_near_clip, float new_far_clip);
			Viewport(const Viewport& vp);
			void operator = (const Viewport& vp);
		};
		// Brush for Widgets
		struct Brush
		{
			c4 color;
		};
		// Define 2d object boundaries
		struct BounderBox2D
		{
			v2 boxLimits = 0;
			BounderBox2D();
			BounderBox2D(float width, float height);
			BounderBox2D(const BounderBox2D& bb);
			void operator=(const BounderBox2D& bb);
			
		};
		// Define 3d object boundaries
		struct BounderBox3D
		{
			v3 boxLimits = 0.0;
			BounderBox3D();
			BounderBox3D(Transform T);
			BounderBox3D(const Transform& T);
			void operator=(const Transform& T);
		};
		// The Arrow is a module that has no transformation
		// and only return direction vectors
		struct Arrow
		{
			//Vector Forward in X direction
			float ForwardVector = 1.0f;
			//Vector Right in Z direction
			float RightVector = 1.0f;
			//Vector Up in Y direction
			float UpVector = 1.0f;
			//Return a v3 with Forward vector;
			v3 ForwardVectorv3();
			//Return a v3 with Right vector;
			v3 RightVectorv3();
			//Return a v3 with Up vector;
			v3 UpVectorv3();
		};
		//Store the properties of the entire application
		//at run time and make it available globally
		struct Global
		{
			class Timer
			{
			public:
				Timer()
				{/**/}
				~Timer()
				{}
				double deltaTime = 0;
				void setTime(double newTime);
				double getTimeSeconds();
			private:
				void setDeltaTime();
				double time = 0;
				double LastFrame = 0, CurrentFrame = 0;
			};
			Global();

		};
	}	
	//Returns a random value between the minimum and the maximum
	float randomInterval(float minInterval, float maxInterval);
	namespace Math
	{
		float radians(float degrees);
		float degrees(float radians);
	}
}
#endif // !RANDOM_RANKIT