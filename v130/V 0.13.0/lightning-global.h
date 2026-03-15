#pragma once
//Random + Lightning
#include "random-rankit.h"
#include "lightning-stream.h"

#include <GLEW/glew.h>
#include <GLFW/glfw3.h>

//Standard
#include <iomanip>

/*								A CLARIFICATION													*/
/*Naming Engine versions works using the final names delta, alpha, beta, release, "Named version", 
which ranges from the most crudeand buggy to the most refined of all, still has the numbering 0.10.0 
which is[GENERATION] - [FEATURE] - [REVIEW]. */
#define LTEMASTERVERSION "0.13.0 DELTA" //	A string with the version number and its usability status
#define LTENUMVERSION 0130			// An integer with the version number

// Estruturas geral do Aplicativo
static Random::Structs::Global::Timer Time;
static Random::Structs::SWindow window(840, 480, "Lightning Engine", true);
static Random::Structs::Context context(4, 3, false);
static Random::Structs::Framebuffers framebuffer(0, false, true);
static Random::Structs::MouseEvents mouse = MouseEvents();
static Random::Structs::Viewport viewport;
//Configuração da Navegação
static double lastX = window.width / 2, lastY = window.height / 2;
static bool fistMouse = true;
static bool isProjection = true;
//Funções da janela
void setInfoTitleBar(bool ShowFPS, bool ShowMousePos);
void showInfoTitleBar(GLFWwindow* wnd, Random::Structs::SWindow window, const Random::Structs::MouseEvents& me);
void InputListener(void(*fn)(GLFWwindow* wnd));
//Set Type shading
void setShading(Shader* shader, solidtext local, int level);