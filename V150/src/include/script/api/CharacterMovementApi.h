#pragma once

#include "../../Node.h"
#include "../../modules/character/CharacterMovementComponent.h"

namespace LightningEngine
{
	// Script-facing bridge to CharacterMovementComponent.
	class CharacterMovementApi
	{
	public:
		static CharacterMovementComponent* Resolve(Node* node)
		{
			if (!node) return nullptr;
			return node->GetComponent<CharacterMovementComponent>();
		}

		static const CharacterMovementComponent* Resolve(const Node* node)
		{
			if (!node) return nullptr;
			return node->GetComponent<CharacterMovementComponent>();
		}

		static bool SetMoveInput(Node* node, float x, float y)
		{
			CharacterMovementComponent* movement = Resolve(node);
			if (!movement) return false;

			movement->SetMoveInput(x, y);
			return true;
		}

		static bool Move(Node* node, Lightning::V2 input)
		{
			CharacterMovementComponent* movement = Resolve(node);
			if (!movement) return false;

			movement->Move(input);
			return true;
		}

		static bool Jump(Node* node)
		{
			CharacterMovementComponent* movement = Resolve(node);
			if (!movement) return false;

			movement->Jump();
			return true;
		}

		static bool IsGrounded(const Node* node)
		{
			const CharacterMovementComponent* movement = Resolve(node);
			if (!movement) return false;

			return movement->IsGrounded();
		}

		static bool SetConfig(Node* node, const CharacterMovementConfig& config)
		{
			CharacterMovementComponent* movement = Resolve(node);
			if (!movement) return false;

			movement->config = config;
			return true;
		}

		static bool GetConfig(const Node* node, CharacterMovementConfig& outConfig)
		{
			const CharacterMovementComponent* movement = Resolve(node);
			if (!movement) return false;

			outConfig = movement->config;
			return true;
		}
	};
}
