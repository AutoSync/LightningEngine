/*

*/

#pragma once

namespace LightningEngine{

class GameInstance 
	{
	public:
		virtual ~GameInstance() = default;
		virtual void Initialize() = 0;
		virtual void Shutdown() = 0;
		virtual void Update(float deltaTime) = 0;
		virtual void Render() = 0;
	};
}
