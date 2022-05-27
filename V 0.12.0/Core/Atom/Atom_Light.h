#ifndef LIGHT_OBJECT_H
#define LIGHT_OBJECT_H

#include "Atom_Model.h"
#define MAX_POINT_LIGHT 4
class LightDirectional
{
private:
	glm::vec3 Direction = glm::vec3(-0.2f, -1.0f, -0.3f);		//Direction Light
	glm::vec3 Diffuse = glm::vec3(0.2f);						//Set the light color Default of 0.2/1.0
	glm::vec3 LightColor = Diffuse;								//Set the light color Default as White
	glm::vec3 Ambient = glm::vec3(0.1f);						//Set the light color Default as 0.0/1.0
	glm::vec3 Specular = glm::vec3(1.0f);						//Set the light color Default as 1.0/1.0
public:
	
	//Get Relative LightDirection
	glm::vec3 getLightDirection()
	{
		return Direction;
	}
	//Set Relative LightDirection 
	void setLightDirection(glm::vec3 NewDirection)
	{
		this->Direction = NewDirection;
	}
	//Set Relative LightDirection 
	void setLightDirection(float NewDirectionX, float NewDirectionY, float NewDirectionZ)
	{
		this->Direction = glm::vec3(NewDirectionX, NewDirectionY, NewDirectionZ);
	}
	//Get Light Color
	glm::vec3 getLightColor()
	{
		return LightColor;
	}
	//Set Light Color in Vec3
	void setLightColor(glm::vec3 NewLightColor)
	{
		this->LightColor = NewLightColor;
	}
	//Set Light Color in X Y Z
	void setLightColor(float x, float y, float z)
	{
		this->LightColor = glm::vec3(x, y, z);
	}
	//Set Light Color in all vectors
	void setLightColor(float all)
	{
		this->LightColor = glm::vec3(all);
	}
	//Get Diffuse Color
	glm::vec3 getDiffuse()
	{
		return Diffuse;
	}
	//Set Diffuse Color in Vec3
	void setDiffuse(glm::vec3 NewDiffuse)
	{
		this->Diffuse = NewDiffuse;
	}
	//Set Diffuse color in X Y Z
	void setDiffuse(float x, float y, float z)
	{
		this->Diffuse = glm::vec3(x, y, z);
	}
	//Set Diffuse color in all vectors
	void setDiffuse(float all)
	{
		this->Diffuse = glm::vec3(all);
	}
	//Set Ambient color
	glm::vec3 getAmbient()
	{
		return Ambient;
	}
	//Set Ambient color in Vec3
	void setAmbient(glm::vec3 NewAmbient)
	{
		this->Ambient = NewAmbient;
	}
	//Set Ambient Color in X Y Z
	void setAmbient(float x, float y, float z)
	{
		this->Ambient = glm::vec3(x, y, z);
	}
	void setAmbient(float all)
	{
		this->Ambient = glm::vec3(all);
	}
	//Get Specular Color
	glm::vec3 getSpecular()
	{
		return Specular;
	}
	//Set Specular Color in Vec3
	void setSpecular(glm::vec3 NewSpecular)
	{
		this->Specular = NewSpecular;
	}
	//Set Specular Color in X Y Z
	void setSpecular(float x, float y, float z)
	{
		this->Specular = glm::vec3(x, y, z);
	}
	//Set Specular Color in All vectors
	void setSpecular(float all)
	{
		this->Specular = glm::vec3(all);
	}

	// Render

	void Render(Shader Shader)
	{
		Shader.use();
		Shader.setVec3("ldir.direction", getLightDirection());
		Shader.setVec3("ldir.diffuse", getDiffuse());
		Shader.setVec3("ldir.specular", getSpecular());
	}
};
class LightPoint 
{
private:

	glm::vec3 Location = glm::vec3(0.0f);
	glm::vec3 Rotation = glm::vec3(0.0f);
	glm::vec3 Scale = glm::vec3(1.0f);
	glm::mat3 Transform = glm::mat3(Location, Rotation, Scale);

	glm::vec3 Diffuse = glm::vec3(1.0f);
	glm::vec3 Ambient = glm::vec3(0.1f);
	glm::vec3 Specular = glm::vec3(1.0f);

	float constant = 1.0f;
	float linear = 0.5f;
	float quadratic = 0.032f;

	glm::mat4 Projection = glm::mat4(1.0f);
	glm::mat4 View = glm::mat4(1.0f);
	glm::mat4 model = glm::mat4(1.0f);

	LightPoint(glm::mat3 SpawnTransform)
	{
		this->Transform = SpawnTransform;
	}
	LightPoint(glm::vec3 Location, glm::vec3 Rotation, glm::vec3 Scale)
	{
		this->Transform = glm::mat3(Location, Rotation, Scale);
	}
};
class SpotLight
{
private:
	glm::vec3 Location = glm::vec3(0.0f);
	glm::vec3 Direction = glm::vec3(0.0f);
	glm::vec3 Diffuse = glm::vec3(1.0f);
	glm::vec3 Ambient = glm::vec3(0.1f);
	glm::vec3 Specular = glm::vec3(1.0f);
	float constant = 1.0f;
	float linear = 0.5f;
	float quadratic = 0.032f;
	float internalCone = 12.f;
	float cutOff = glm::cos(glm::radians(internalCone));
	float externalCone = 15.0f;
	float outerCutOff = glm::cos(glm::radians(externalCone));

public:

	SpotLight(CameraObject Camera, glm::vec3 SpawnLocation)
	{
		setDirection(glm::vec3(Camera.Front));
		setLocation(SpawnLocation);
	}
	SpotLight(glm::vec3 SpawnLocation)
	{
		setLocation(SpawnLocation);
	}

	//Get Diffuse Color
	glm::vec3 getDiffuse()
	{
		return Diffuse;
	}
	//Set Diffuse Color in Vec3
	void setDiffuse(glm::vec3 NewDiffuse)
	{
		Diffuse = NewDiffuse;
	}
	//Set Diffuse color in X Y Z
	void setDiffuse(float x, float y, float z)
	{
		Diffuse = glm::vec3(x, y, z);
	}
	//Set Diffuse color in all vectors
	void setDiffuse(float all)
	{
		Diffuse = glm::vec3(all);
	}
	//Set Ambient color
	glm::vec3 getAmbient()
	{
		return Ambient;
	}
	//Set Ambient color in Vec3
	void setAmbient(glm::vec3 NewAmbient)
	{
		Ambient = NewAmbient;
	}
	//Set Ambient Color in X Y Z
	void setAmbient(float x, float y, float z)
	{
		Ambient = glm::vec3(x, y, z);
	}
	//Set Ambient color in all vectors
	void setAmbient(float all)
	{
		Ambient = glm::vec3(all);
	}
	//Get Specular Color
	glm::vec3 getSpecular()
	{
		return Specular;
	}
	//Set Specular Color in Vec3
	void setSpecular(glm::vec3 NewSpecular)
	{
		Specular = NewSpecular;
	}
	//Set Specular Color in X Y Z
	void setSpecular(float x, float y, float z)
	{
		Specular = glm::vec3(x, y, z);
	}
	//Set Specular Color in All vectors
	void setSpecular(float all)
	{
		Specular = glm::vec3(all);
	}
	//Get Constant Value
	float getConstant()
	{
		return constant;
	}
	//Set Constant Value
	void setConstant(float NewConstant)
	{
		constant = NewConstant;
	}
	//Get Linear Value
	float getLinear()
	{
		return linear;
	}
	//Set Linear Value
	void setLinear(float NewLinear)
	{
		linear = NewLinear;
	}
	//Get Quadratic Value
	float getQuadratic()
	{
		return quadratic;
	}
	//Set Quadratic Value
	void setQuadratic(float NewQuadratic)
	{
		quadratic = NewQuadratic;
	}
	//Get Internal cone value
	float getInternalCone()
	{
		return internalCone;
	}
	//Get External cone value
	float getExternalCone()
	{
		return externalCone;
	}
	//Set Angle internal Cone
	void setInternalCone(float AngleCone)
	{
		internalCone = AngleCone;
	}
	//Set Angle External cone
	void setExternalCone(float AngleCone)
	{
		externalCone = AngleCone;
	}
	//TRANSFORM

	//get Relative Location
	glm::vec3 getLocation()
	{
		return Location;
	}
	//Set Absolute Location in Vec3
	void setLocation(glm::vec3 NewLocation)
	{
		Location = NewLocation;
	}
	//Set Absolute Location in X Y Z
	void setLocation(float x, float y, float z)
	{
		Location = glm::vec3(x, y, z);
	}
	//Set Absolute Location in All vectors
	void serLocation(float all)
	{
		Location = glm::vec3(all);
	}
	//Get Direction value
	glm::vec3 getDirection()
	{
		return Direction;
	}
	//Set Direction Value in Vec3
	void setDirection(glm::vec3 NewDirection)
	{
		Direction = NewDirection;
	}
	//Set Direction Value in X Y Z
	void setDirection(float x, float y, float z)
	{
		Direction = glm::vec3(x, y, z);
	}
	//Set Direction Value in All Vectors
	void setDirection(float all)
	{
		Direction = glm::vec3(all);
	}
};
#endif // !LIGHT_OBJECT_H