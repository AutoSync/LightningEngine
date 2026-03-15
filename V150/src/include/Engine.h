// Engine — Version and future subsystem registry.
// The game loop lives in Window::Run(). This class is reserved for
// higher-level subsystem management (Audio, Physics, Networking, etc.)
// as those systems are implemented.
//
// To start a game:
//   MyGame game;
//   game.Run("Title", 1280, 720);

#pragma once

namespace LightningEngine {

	class Engine {
	public:
		static const char* Version() { return "0.15.0-dev"; }
	};

}
