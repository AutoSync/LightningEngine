#include "Element_static_model.h"

void StaticModel::setLocation(glm::vec3 NewLocation)
{
	this->Location = NewLocation;
}

void StaticModel::setLocation(float NewLocationX, float NewLocationY, float NewLocationZ)
{
	this->Location = glm::vec3(NewLocationX, NewLocationY, NewLocationZ);
}

void StaticModel::setScale(glm::vec3 NewScale)
{
	this->Scale = NewScale;
}

void StaticModel::setScale(float NewScaleX, float NewScaleY, float NewScaleZ)
{
	this->Scale = glm::vec3(NewScaleX, NewScaleY, NewScaleZ);
}

void StaticModel::setScale(float AllAxis)
{
	this->Scale = glm::vec3(AllAxis);
}

void StaticModel::Draw(Model& model, Shader& shader)
{
	glm::mat4 Mesh = glm::mat4(1.0f);
	Mesh = glm::translate(Mesh, Location);
	Mesh = glm::scale(Mesh, Scale);
	shader.setMat4("model", Mesh);
	model.Draw(shader);
}
