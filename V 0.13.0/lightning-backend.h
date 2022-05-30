#pragma once
#include "random-rankit.h"
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

//Standard Library
#include <iostream>
#include <chrono>
#include <string>


// Lightning Bool
#define LFALSE			 0x0000		// Lightning Bool False
#define LTRUE			 0x0001		// Lightning Bool True
#define LDEFAULT		 0x0002		// Lightning Default Command
//Lightning Rendering
#define LR_DEPTH		0x0003	//Flag * Rendering Depth
#define DEPTH_ALWAYS	4		//Function fn The depth test always passes.
#define DEPTH_NEVER		5		//Function fn The depth test never passes.
#define DEPTH_LESS		6		//Function fn Passes if depth < Buffer stored
#define DEPTH_EQUAL		7		//Function fn Passes if depth == Buffer stored
#define DEPTH_LEQUAL	8		//
#define DEPTH_GREATER	9
#define DEPTH_NOTEQUAL	10 
#define DEPTH_GEQUAL	11
#define DEPTH_NOMASK	12		//Condition * disable mask depth
#define LR_STENCIL		200
#define LR_BLEND		201
#define LR_CULL			202
#define LR_CULL_BACK	203
#define LR_CULL_CCW		204
#define LR_				205

//Lightning Console Log
using namespace std::chrono;
typedef steady_clock::time_point now;

class Console
{
public:
	Console();
	~Console();
	void init();
	void Log(std::string text);
	void Msg(std::string text);
	void Now(std::string text);
private:
	std::string durationProcess(now Start, now End);
	now Start;
	now End;
};

//Lightning Rendering Command
//Pass rendering commands using a flag and a condition 
//and the bakend takes care of processing the command
void RenderCommand(int flag, int condition);
//Rotates rendered Lightning objects
glm::vec3 MakeRotate(glm::vec3 rotation);