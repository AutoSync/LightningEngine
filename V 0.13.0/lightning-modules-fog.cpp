#include "lightning-modules-fog.h"

Lightning::Modules::Fog::Fog()
{
    this->fogNear = 0.1f;
    this->fogFar = 100.0f;
    this->colorFog = c3(255);
    this->density = 1.0f;
}

Lightning::Modules::Fog::Fog(float newNear, float newFar)
{
    this->fogNear = newNear;
    this->fogFar = newFar;
    this->colorFog = c3(255);
    this->density = 1.0f;
}

Lightning::Modules::Fog::~Fog()
{
}

void Lightning::Modules::Fog::use(Shader* shader)
{
    if (visibility)
    {
        shader->setBool("fogVisible", visibility);
        shader->setVec3("colorFog", colorFog);
        shader->setFloat("fogDensity", density);
        shader->setFloat("fogNear", fogNear);
        shader->setFloat("fogFar", fogFar);
    }
    else
    {
        shader->setBool("fogVisible", visibility);
    }
}

void Lightning::Modules::Fog::setColorFog(c3 newColor)
{
    this->colorFog = newColor;
}

void Lightning::Modules::Fog::setColorFog(int R, int G, int B)
{
    this->colorFog.r = R;
    this->colorFog.g = G;
    this->colorFog.b = B;
}

c3 Lightning::Modules::Fog::getColorFog()
{
    return this->colorFog;
}

void Lightning::Modules::Fog::setDensityFog(float newDensity)
{
    this->density = newDensity;
}

void Lightning::Modules::Fog::setStartFog(float distance)
{
    this->fogNear = distance;
}

void Lightning::Modules::Fog::setEndFog(float distance)
{
    this->fogFar = distance;
}
