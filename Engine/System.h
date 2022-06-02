#pragma once
#include "Types.h"
#include "GameTime.h"
// The system header defines the application data

//Software Version
#define APP_VERSION_STRING "0.14.0.0"
using namespace Lightning;

static EngineSettings Settings(840, 480, "Lightning Engine", true);
static MouseEvents mouse = MouseEvents();
GameTime Time = GameTime();
//navigation settings
static double lastX = Settings.width / 2, lastY = Settings.height / 2;
static bool firstMouse = true;
static bool isProjection = true;
//Rendering by System
static glm::mat4 model = glm::mat4(1.0f), view = glm::mat4(1.0f), projection = glm::mat4(1.0f);