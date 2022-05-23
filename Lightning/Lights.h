#pragma once
#include "Types.h"
#include "SceneComponent.h"

namespace Lightning
{
	class Light : SceneComponent
	{
	public:
		Light();
		
	};

	class Point : Light
	{
	public:
		
	public:
		Point();
		Point(Transform T);
	private:
		void InitializeComponent(Transform T);
	};
}

