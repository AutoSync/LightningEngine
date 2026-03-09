// Class that draws the game level and has the entire hierarchy of Nodes
// -------------------------- MAP OF THE CLASSES GAME --------------------------
// ------ WINDOW - GAMEINSTANCE - [ LEVEL ] - NODES - COMPONENTS ---------------
// ----------------------------------------------------------------------------- 
#pragma once

#include <SDL3/SDL.h>
namespace LightningEngine {
	class Level {
	public:
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void Update(float dt) = 0;
		virtual void Render() = 0;
	};
}
