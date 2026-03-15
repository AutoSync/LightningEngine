#include "Atom_model.h"

class StaticModel
{
private:
	glm::vec3 Location = glm::vec3(1.0f);
	glm::vec3 Scale = glm::vec3(1.0f);
public:
	void setLocation(glm::vec3 NewLocation);
	void setLocation(float NewLocationX, float NewLocationY, float NewLocationZ);
	void setScale(glm::vec3 NewScale);
	void setScale(float NewScaleX, float NewScaleY, float NewScaleZ);
	void setScale(float AllAxis);
	void Draw(Model& model, Shader& shader);
};

