#ifndef LEVEL_LTE
#define LEVEL_LTE

#include "../../Engine/EngineObjects.h"

//Shaders
const char ShaderStd[] = "../Shaders/StandardShader.vert";
LightningShader Shaders[] =
{
	LightningShader(ShaderStd, "BoatScene.frag")
};
//Objects
Model Ship[] =
{
	Model("")
};

#endif // !LEVEL_H

