#pragma once
#include "Types.h"

// The system header defines the application data

//Software Version
#define APP_VERSION_STRING "0.14.0.0"
// Estruturas geral do Aplicativo
EngineSettings Settings = EngineSettings(840, 480, "Lightning Engine", true);
MouseEvents mouse = MouseEvents();
//static Random::Structs::Viewport viewport;
//Configuração da Navegação
static double lastX = Settings.width / 2, lastY = Settings.height / 2;
static bool fistMouse = true;
static bool isProjection = true;
//Funções da janela
//void setInfoTitleBar(bool ShowFPS, bool ShowMousePos);
//void showInfoTitleBar(GLFWwindow* wnd, Random::Structs::SWindow window, const Random::Structs::MouseEvents& me);
//void InputListener(void(*fn)(GLFWwindow* wnd));