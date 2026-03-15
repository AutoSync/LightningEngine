#include "lightning-modules-lights.h"

Lightning::Modules::Lights::Lights()
{
}

Lightning::Modules::Lights::Lights(const Lightning::Front::ELightMode LightMode)
{
	switch (LightMode)
	{
	case Lightning::Front::ELightMode::LIGHT_DIRECT:
		createLightDirection();
		break;
	case Lightning::Front::ELightMode::LIGHT_POINT:
		createLightPoint();
		break;
	case Lightning::Front::ELightMode::LIGHT_SPOTLIGHT:
		createSpotLight();
		break;
	}
}

Lightning::Modules::Lights::~Lights()
{
}

void Lightning::Modules::Lights::createLightDirection()
{
	this->LightMode = Lightning::Front::ELightMode::LIGHT_DIRECT;
	this->transform.Location = v3(-0.2f, -1.0f, -0.3f);
	this->diffuse = c3(255);
	this->ambient = c3(50);
	this->specular = c3(255);
}

void Lightning::Modules::Lights::createLightPoint()
{
	this->LightMode = Lightning::Front::ELightMode::LIGHT_POINT;
	this->transform.Location = v3(0.0f, 0.0f, 0.0f);
	this->diffuse = c3(255);
	this->ambient = c3(0);
	this->specular = c3(255);
	this->constant = 1.0f;
	this->linear = 0.5f;
	this->quadratic = 0.032f;
}

void Lightning::Modules::Lights::createSpotLight()
{
}


void Lightning::Modules::Lights::useLight(Shader* shader)
{
	switch (LightMode)
	{
	case Lightning::Front::ELightMode::LIGHT_DIRECT:
		shader->setVec3("LD.direction", this->transform.Location);
		shader->setVec3("LD.diffuse", this->diffuse);
		shader->setVec3("LD.ambient", this->ambient);
		shader->setVec3("LD.specular", this->specular);
		break;
	case Lightning::Front::ELightMode::LIGHT_POINT:
		shader->setVec3("LP.Location", this->transform.Location);
		shader->setVec3("LP.diffuse", this->diffuse);
		shader->setVec3("LP.ambient", this->ambient);
		shader->setVec3("LP.specular", this->specular);
		shader->setFloat("LP.constant", this->constant);
		shader->setFloat("LP.linear", this->linear);
		shader->setFloat("LP.quadratic", this->quadratic);
		break;
	case Lightning::Front::ELightMode::LIGHT_SPOTLIGHT:
		break;
	}
}

void Lightning::Modules::Lights::useLight(int iterator, Shader* shader)
{
	switch (LightMode)
	{
	case Lightning::Front::ELightMode::LIGHT_POINT:
		shader->setVec3(shader->matrixString("LP", iterator, "location"), this->transform.Location);
		shader->setVec3(shader->matrixString("LP", iterator, "diffuse"), this->diffuse);
		shader->setVec3(shader->matrixString("LP", iterator, "ambient"), this->ambient);
		shader->setVec3(shader->matrixString("LP", iterator, "specular"), this->specular);
		shader->setFloat(shader->matrixString("LP", iterator, "constant"), this->constant);
		shader->setFloat(shader->matrixString("LP", iterator, "linear"), this->linear);
		shader->setFloat(shader->matrixString("LP", iterator, "quadratic"), this->quadratic);
		break;
	}
}

void Lightning::Modules::Lights::setVisibility(bool newVisibility)
{
}

void Lightning::Modules::Lights::setLightMode(Lightning::Front::ELightMode lightmode)
{
	this->LightMode = lightmode;
}

void Lightning::Modules::Lights::setTransform(Transform newT)
{
	this->transform = newT;
}

Transform Lightning::Modules::Lights::getTransform()
{
	return transform;
}

void Lightning::Modules::Lights::setRelativeLocation(v3 NewLocation)
{
	this->transform.Location = NewLocation;
}

v3 Lightning::Modules::Lights::getRelativeLocation()
{
	return this->transform.Location;
}

void Lightning::Modules::Lights::setRelativeRotation(r3 NewRotation)
{
	this->transform.Rotation = NewRotation;
}

r3 Lightning::Modules::Lights::getRelativeRotation()
{
	return this->transform.Rotation;
}

void Lightning::Modules::Lights::setRelativeScale(v3 NewScale)
{
	this->transform.Scale = NewScale;
}

v3 Lightning::Modules::Lights::getRelativeScale()
{
	return this->transform.Scale;
}

void Lightning::Modules::Lights::setLightColor(c3 newColor)
{
	this->diffuse = newColor;
}

c3 Lightning::Modules::Lights::getLightColor()
{
	return diffuse;
}

void Lightning::Modules::Lights::setAmbientColor(c3 newAmbient)
{
	this->ambient = newAmbient;
}

c3 Lightning::Modules::Lights::getAmbientColor()
{
	return ambient;
}

void Lightning::Modules::Lights::setSpecularColor(c3 newSpecular)
{
	this->specular = newSpecular;
}

c3 Lightning::Modules::Lights::getSpecularColor()
{
	return specular;
}

void Lightning::Modules::Lights::setQuadratic(float newQuadratic)
{
	this->quadratic = newQuadratic;
}

float Lightning::Modules::Lights::getQuadratic()
{
	return this->quadratic;
}

void Lightning::Modules::Lights::setLinear(float newLinear)
{
	this->linear = newLinear;
}

float Lightning::Modules::Lights::getLinear()
{
	return linear;
}
