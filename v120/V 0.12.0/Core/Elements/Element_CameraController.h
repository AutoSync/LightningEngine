#pragma once
#include "../Atom/Atom_camera.h"
class CameraController
{

public:
	CameraObject ActiveCamera;
	CameraController(CameraObject Camera);
	CameraObject getActiveCamera();
	void setActiveCamera(CameraObject Camera);
};

