#include "Element_CameraController.h"

CameraController::CameraController(CameraObject Camera)
{
	this->ActiveCamera = Camera;
}

CameraObject CameraController::getActiveCamera()
{
	return this->ActiveCamera;
}
