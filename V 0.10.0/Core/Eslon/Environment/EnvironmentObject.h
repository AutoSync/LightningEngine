#ifndef LIGHTNING_ENVIRONMENT_OBJECT_H
#define LIGHTNING_ENVIRONMENT_OBJECT_H
// Libary Standard
#include <iostream> 
// OPENGL MATH LIBARY
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "CameraObject.h"

enum MOBILITY
{
	STATIC,
	DYNAMIC,
	MOVABLE
};

class environmentObject
{
private:
	bool Visibility = true;						//Set if object is visible
	bool VisibleOnGame = true;					//Set if object is visible when game is running
	glm::vec3 Location = glm::vec3(0.0f);		// Absolute Location in the world
	glm::vec3 Rotation = glm::vec3(0.0f);		// Absolute Rotation in the world
	glm::vec3 Scale = glm::vec3(1.0f);			// Absolute Scale in the world
	glm::mat3 Transform = glm::mat3(Location, Rotation, Scale);	//Matrix gathering location, rotation and scale
	MOBILITY mobility = MOVABLE;				//Set Mobility the object
	CameraObject Camera = glm::vec3(0.0f);		//Camera 
public:
	//Get if the object is visible
	bool getVisibility()
	{
		return Visibility;
	}
	//Set if object is visible
	void setVisibility(bool NewVisibility)
	{
		Visibility = NewVisibility;
	}
	// Toggles visibility
	void toggleVisibility()
	{
		if (Visibility == true)
			Visibility = false;
		else if(Visibility == false)
			Visibility = true;
	}
	//Get the current mobility of the object
	MOBILITY getMobility()
	{
		return mobility;
	}
	//Set the Mobility the object
	void setMobility(MOBILITY NewMobility)
	{
		mobility = NewMobility;
	}
	//Get object the camera
	CameraObject getCamera()
	{
		return Camera;
	}
	void setCamera(CameraObject NewCamera)
	{
		Camera = NewCamera;
	}
	//Get absolute Location this object
	glm::vec3 getLocation()
	{
		return Location;
	}
	//Set a new Location
	void setLocation(glm::vec3 NewLocation)
	{
		if (mobility == MOVABLE)
			Location = NewLocation;
		else
			std::cout << "CANNOT MOVE OBJECTS WHEN STATIC OR DYNAMIC" << std::endl;
	}
	//Gets the Current Rotation
	glm::vec3 getRotation()
	{
		return Rotation;
	}
	//set a new Rotation
	void setRotation(glm::vec3 NewRotation)
	{
		if(mobility == MOVABLE)
			Rotation = NewRotation;
		else
			std::cout << "CANNOT ROTATE OBJECTS WHEN STATIC OR DYNAMIC" << std::endl;
	}
	//Gets the current scale
	glm::vec3 getScale()
	{
		return Scale;
	}
	//Set a new Scale
	void setScale(glm::vec3 NewScale)
	{
		if(mobility == MOVABLE)
			Scale = NewScale;
		else
			std::cout << "CANNOT SCALE OBJECTS WHEN STATIC OR DYNAMIC" << std::endl;
	}
	//Return the matrix 3 dimensions of Transform
	glm::mat3 getTransform()
	{
		return Transform;
	}
	//Set a new matrix transform
	void setTransform(glm::mat3 NewTransform)
	{
		if (mobility == MOVABLE)
			Transform = NewTransform;
		else
			std::cout << "CANNOT TRANSFORM OBJECTS WHEN STATIC OR DYNAMIC" << std::endl;
	}

};
#endif
