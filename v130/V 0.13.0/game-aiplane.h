#pragma once
#include "random-rankit.h"
#include "lightning-modules-camera.h"
#include "lightning-model.h"
#include "equinox-bake-opengl.h"

using namespace Lightning::Modules;
using namespace Lightning::Model;
class GAirplane
{
public:
	GAirplane(HardMesh* NewAirplane, Shader* NewAirplaneShader, Camera* NewAirplaneCamera);
	~GAirplane(){ /*Destructor*/ }
	void Render();
	void throtle(float input);
	void steer(float input);
	void HeightControl(float input);
	v3 cameraOffset = v3(-3.0f, 1.0f, 0.0f);
	float forwardDirection = 0;
	float cameraPitch = -0.5f;
private:
	HardMesh* airplane;
	Shader* shader;
	Camera* camera;
};
