//Cabeçalho Padrão
#ifndef STDLIB
#define STDLIB
#include <iostream>
#include <string>
using namespace std;
#endif // !STDLIB
//OPENGL
#ifndef GL
#define GL
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif // !GL
//OPENGL GL MATH
#ifndef GLMATH
#define GLMATH
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#endif // !GLMATH
//ENGINE ATOM OBJECTS
#ifndef LE_ATOM_OBJ
#define LE_ATOM_OBJ
#include "Core/Atom/Atom_print.h"
#include "Core/Atom/Atom_Shader.h"
#include "Core/Atom/Atom_Camera.h"
#include "Core/Atom/Atom_texture.h"
#include "Core/Atom/Atom_light.h"
#include "Core/Atom/Atom_model.h"
#endif // !LE_ATOM_OBJ
//ENGINE ELEMENTS OBJECTS
#ifndef LE_ELEMENT_OBJ
#define LE_ELEMENT_OBJ
#include "Core/Elements/Element_CameraController.h"
#endif // !LE_ELEMENT_OBJ






