#pragma once

namespace LightningEngine
{
	// Tunables for 2D character locomotion.
	struct CharacterMovementConfig
	{
		float maxSpeed      = 320.0f;
		float acceleration  = 2400.0f;
		float deceleration  = 2000.0f;
		float jumpForce     = 620.0f;
		float gravityScale  = 1.0f;
		float airControl    = 0.35f;
		float maxFallSpeed  = 1800.0f;

		// Demo ground plane used when no collision world is wired yet.
		float groundPlaneY  = 0.0f;
		bool  startGrounded = true;
	};
}
