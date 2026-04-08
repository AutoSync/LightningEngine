#pragma once

#include "../../Component.h"
#include "../../Math.h"
#include "CharacterMovementConfig.h"

namespace LightningEngine
{
	class CharacterMovementComponent : public Component
	{
	public:
		CharacterMovementConfig config;

		// Public read-only state for debug/inspector demos.
		Lightning::V2 velocity = { 0.0f, 0.0f };

		void OnAttach() override;
		void Update(float dt) override;

		void Move(Lightning::V2 input);
		void SetMoveInput(float x, float y);
		void Jump();
		bool IsGrounded() const;

	private:
		Lightning::V2 moveInput = { 0.0f, 0.0f };
		bool grounded = false;
		const float baseGravity = 1200.0f;
	};
}
