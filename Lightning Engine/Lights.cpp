#include "Lights.h"

Lightning::Light::Light()
{
	Transform T;
	LightType type = LightType::Direct;
	InitializeLight(T, type);
}

Lightning::Light::Light(Transform T, LightType type)
{
	InitializeLight(T, type);
}

void Lightning::Light::Render(Shader* shader)
{
	switch (Type)
	{
	case Lightning::Direct:
		shader->SetV3("LD.direction", this->transform.Position);
		shader->SetColor("LD.diffuse", this->diffuse);
		shader->SetColor("LD.ambient", this->ambient);
		shader->SetColor("LD.specular", this->specular);
		break;
	case Lightning::Spotlight:
		break;
	case Lightning::Point:
		shader->SetV3("LP.Location", this->transform.Position);
		shader->SetColor("LP.diffuse", this->diffuse);
		shader->SetColor("LP.ambient", this->ambient);
		shader->SetColor("LP.specular", this->specular);
		shader->SetFloat("LP.constant", this->constant);
		shader->SetFloat("LP.linear", this->linear);
		shader->SetFloat("LP.quadratic", this->quadratic);
		break;
	case Lightning::Rect:
		break;
	}
}

void Lightning::Light::Render(int iterator, Shader* shader)
{
	switch (Type)
	{
	case Lightning::Direct:
		shader->SetV3("LD.direction", this->transform.Position);
		shader->SetColor("LD.diffuse", this->diffuse);
		shader->SetColor("LD.ambient", this->ambient);
		shader->SetColor("LD.specular", this->specular);
		break;
	case Lightning::Spotlight:
		break;
	case Lightning::Point:
		shader->SetV3(shader->MatrixChars("LP", iterator, "location"), this->transform.Position);
		shader->SetColor(shader->MatrixChars("LP", iterator, "diffuse"), this->diffuse);
		shader->SetColor(shader->MatrixChars("LP", iterator, "ambient"), this->ambient);
		shader->SetColor(shader->MatrixChars("LP", iterator, "specular"), this->specular);
		shader->SetFloat(shader->MatrixChars("LP", iterator, "constant"), this->constant);
		shader->SetFloat(shader->MatrixChars("LP", iterator, "linear"), this->linear);
		shader->SetFloat(shader->MatrixChars("LP", iterator, "quadratic"), this->quadratic);
		break;
	case Lightning::Rect:
		break;
	}
}

void Lightning::Light::CreateDirectionLight()
{
}

void Lightning::Light::CreateSpotlight()
{
}

void Lightning::Light::CreatePoint()
{
}

void Lightning::Light::SetLightColor(C3 Color)
{
	this->diffuse = Color;
}

void Lightning::Light::SetAmbientColor(C3 Color)
{
	this->ambient = Color;
}

void Lightning::Light::InitializeLight(Transform T, LightType type)
{
	this->Type = type;
	this->transform = T;
	switch (type)
	{
	case Lightning::Direct:
		CreateDirectionLight();
		break;
	case Lightning::Spotlight:
		CreateSpotlight();
		break;
	case Lightning::Point:
		CreatePoint();
		break;
	case Lightning::Rect:
		break;
	}
}
