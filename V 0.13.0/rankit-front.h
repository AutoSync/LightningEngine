#pragma once
#include "random-rankit.h"
//Random library namespace
using namespace Random::Structs;
namespace Random
{
	//Front-end library namespace, images, graphics, buttonsand widgets
	namespace Front
	{
		//Main class of all widgets
		class Widget
		{
		public:
			Widget(MouseEvents ME);
			~Widget();
		private:
			MouseEvents* me;
		private:
			bool onMouseHover();
			bool onClick(void(*_function));
		};

		class Button
		{
		public:
			Button(text Text);
			~Button();
		private:
			void setColor(v3 NewColor);
		};


	}
}
