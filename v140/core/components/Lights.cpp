#include "../../Lights.h"

LightningEngine::Light::Light()
{
	Transform T;
	LightType type = LightType::Directional;
	InitializeLight(T, type);
}

LightningEngine::Light::Light(Transform T, LightType type)
{
	InitializeLight(T, type);
}

void LightningEngine::Light::Render(Shader* shader)
{
	switch (Type)
	{
	case LightningEngine::Directional:
		shader->SetV3("LD.direction", this->transform.Position);
		shader->SetColor("LD.diffuse", this->diffuse);
		shader->SetColor("LD.ambient", this->ambient);
		shader->SetColor("LD.specular", this->specular);
		break;
	case LightningEngine::Spotlight:
		break;
	case LightningEngine::Point:
		shader->SetV3("LP.Location", this->transform.Position);
		shader->SetColor("LP.diffuse", this->diffuse);
		shader->SetColor("LP.ambient", this->ambient);
		shader->SetColor("LP.specular", this->specular);
		shader->SetFloat("LP.constant", this->constant);
		shader->SetFloat("LP.linear", this->linear);
		shader->SetFloat("LP.quadratic", this->quadratic);
		break;
	case LightningEngine::Rect:
		break;
	}
}

void LightningEngine::Light::Render(int iterator, Shader* shader)
{
	switch (Type)
	{
	case LightningEngine::Directional:
		shader->SetV3("LD.direction", this->transform.Position);
		shader->SetColor("LD.diffuse", this->diffuse);
		shader->SetColor("LD.ambient", this->ambient);
		shader->SetColor("LD.specular", this->specular);
		break;
	case LightningEngine::Spotlight:
		break;
	case LightningEngine::Point:
		shader->SetV3(shader->MatrixChars("LP", iterator, "location"), this->transform.Position);
		shader->SetColor(shader->MatrixChars("LP", iterator, "diffuse"), this->diffuse);
		shader->SetColor(shader->MatrixChars("LP", iterator, "ambient"), this->ambient);
		shader->SetColor(shader->MatrixChars("LP", iterator, "specular"), this->specular);
		shader->SetFloat(shader->MatrixChars("LP", iterator, "constant"), this->constant);
		shader->SetFloat(shader->MatrixChars("LP", iterator, "linear"), this->linear);
		shader->SetFloat(shader->MatrixChars("LP", iterator, "quadratic"), this->quadratic);
		break;
	case LightningEngine::Rect:
		break;
	}
}

void LightningEngine::Light::CreateDirectionLight()
{
}

void LightningEngine::Light::CreateSpotlight()
{
}

void LightningEngine::Light::CreatePoint()
{
}

void LightningEngine::Light::SetLightColor(C3 Color)
{
	this->diffuse = Color;
}

void LightningEngine::Light::SetAmbientColor(C3 Color)
{
	this->ambient = Color;
}

void LightningEngine::Light::InitializeLight(Transform T, LightType type)
{
	this->Type = type;
	this->transform = T;
	switch (type)
	{
	case LightningEngine::Directional:
		CreateDirectionLight();
		break;
	case LightningEngine::Spotlight:
		CreateSpotlight();
		break;
	case LightningEngine::Point:
		CreatePoint();
		break;
	case LightningEngine::Rect:
		break;
	}
}
