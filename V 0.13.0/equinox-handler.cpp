#include "equinox-handler.h"

void Equinox::Handler::free()
{
	this->VertexShader = "";
	this->FragmentShader = "";
}

void Equinox::Handler::setRHI(RHI new_RHI)
{
	this->RenderHardwareInterface = new_RHI;
}

