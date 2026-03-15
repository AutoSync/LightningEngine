#include "game-aiplane.h"


GAirplane::GAirplane(HardMesh* NewAirplane, Shader* NewAirplaneShader, Camera* NewAirplaneCamera)
{
	airplane = NewAirplane;
	shader = NewAirplaneShader;

	camera = NewAirplaneCamera;
	v3 temp = airplane->getLocation() + cameraOffset;
	camera->setLocation(temp.x, temp.y, temp.z);
	camera->SetRotation(forwardDirection, cameraPitch, 0.0f);
}
void GAirplane::Render()
{
	airplane->Draw(shader);
}


void GAirplane::throtle(float input)
{
	v3 temp;
	Arrow arrow;
	temp = this->airplane->getLocation() + arrow.ForwardVectorv3() * input;
	this->airplane->setLocation(temp);
	
	//Update Camera
	v3 ctemp;
	ctemp += temp + cameraOffset;
	this->camera->setLocation(ctemp.x, ctemp.y, ctemp.z);
	this->camera->SetRotation(forwardDirection, cameraPitch, 0.0f);

}

void GAirplane::steer(float input)
{
	v3 temp;
	Arrow arrow;
	temp = this->airplane->getLocation() + arrow.RightVectorv3() * input;
	this->airplane->setLocation(temp);

	v3 ctemp;
	ctemp += temp + cameraOffset;
	this->camera->setLocation(ctemp.x, ctemp.y, ctemp.z);
	this->camera->SetRotation(forwardDirection, cameraPitch, 0.0f);
}

void GAirplane::HeightControl(float input)
{
	v3 temp;
	Arrow arrow;
	temp = this->airplane->getLocation() + arrow.UpVectorv3() * input;
	this->airplane->setLocation(temp);

	v3 ctemp;
	ctemp += temp + cameraOffset;
	this->camera->setLocation(ctemp.x, ctemp.y, ctemp.z);
	this->camera->SetRotation(forwardDirection, 0.0f, 0.0f);
}


