// GameInstance — Abstract base class for user games.
// Inherits the full engine lifecycle from Window.
// Users extend this class and override Initialize/Shutdown/Update/Render.
//
// CLASS HIERARCHY:
// WINDOW -> [ GAMEINSTANCE ] -> LEVEL -> NODES -> COMPONENTS

#pragma once
#include "Window.h"

namespace LightningEngine {

	class GameInstance : public Window {
	public:
		virtual ~GameInstance() = default;

		// Initialize, Shutdown, Update(float), Render — inherited from Window.
		// All four must be implemented by the subclass.
	};

}
