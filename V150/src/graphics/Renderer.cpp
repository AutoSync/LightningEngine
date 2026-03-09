#include "../include/Renderer.h"
#include <iostream>

namespace LightningEngine{

	Renderer::Renderer(SDL_Window* window)
	{
		renderer = SDL_CreateRenderer(window, nullptr);
		if (!renderer)
		{
			std::cerr << "Renderer could not be created! SDL Error: " << SDL_GetError() << std::endl;
		}
	}

	Renderer::~Renderer()
	{
		if(renderer)
		{
			SDL_DestroyRenderer(renderer);
		}
	}

	void Renderer::Clear()
	{
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
	}

	void Renderer::Present()
	{
		SDL_RenderPresent(renderer);
	}

	void Renderer::SetDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
	{
		SDL_SetRenderDrawColor(renderer, r, g, b, a);
	}

	void Renderer::DrawRect(float x, float y, float w, float h)
	{
		SDL_FRect rect = { x, y, w, h };
		SDL_RenderRect(renderer, &rect);
	}

	void Renderer::FillRect(float x, float y, float w, float h)
	{
		SDL_FRect rect = { x, y, w, h };
		SDL_RenderFillRect(renderer, &rect);
	}
}