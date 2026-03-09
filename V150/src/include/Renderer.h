#pragma once
#include <SDL3/SDL.h>
#include <memory>

namespace LightningEngine {
	class Renderer
	{
	private:
		SDL_Renderer* renderer;
	public:
		Renderer(SDL_Window* window);
		~Renderer();

		void Clear();
		void Present();
		void SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a);
		void DrawRect(float x, float y, float w, float h);
		void FillRect(float x, float y, float w, float h);

		SDL_Renderer* GetSDLRenderer() { return renderer; }
	};
}
