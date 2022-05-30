#pragma once
#include "Atom_camera.h"
class CameraController
{

public:
	CameraObject ActiveCamera;
	CameraController(CameraObject Camera);
	CameraObject getActiveCamera();
	void setActiveCamera(CameraObject Camera);
};

