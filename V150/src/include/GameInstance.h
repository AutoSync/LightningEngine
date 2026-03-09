/*

*/
#pragma once
#include <SDL3/SDL.h>

namespace LightningEngine{

class GameInstance 
	{
	public:
		GameInstance() { init(); };
		const char* title;
		int width, height;
		virtual ~GameInstance() = default;
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
		SDL_Renderer* getSDLRenderer() const { return renderer; }
	private:
		void init();
		const char* rendererName;
		SDL_Renderer* renderer;
		SDL_Window* window;
	};
}
