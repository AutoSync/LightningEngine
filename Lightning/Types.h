#pragma once
#include <cstdio>
#include <string>
#include <vector>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;

namespace Lightning
{
	//Typedefs
	
	//
	typedef unsigned int uint;

	//Structs

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
	float Clampf(float value, float min, float max);
}