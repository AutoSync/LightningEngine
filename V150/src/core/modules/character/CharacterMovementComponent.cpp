#include "../../../include/modules/character/CharacterMovementComponent.h"
#include "../../../include/Node.h"

namespace LightningEngine
{
    void CharacterMovementComponent::OnAttach()
    {
        grounded = config.startGrounded;
        moveInput = { 0.0f, 0.0f };
        velocity = { 0.0f, 0.0f };
    }

    void CharacterMovementComponent::Update(float dt)
    {
        if (!owner) return;

        const float dtSec = dt * 0.001f;
        if (dtSec <= 0.0f) return;

        const float control = grounded ? 1.0f : config.airControl;
        const float targetVX = moveInput.x * config.maxSpeed;

        if (LMath::Abs(targetVX) > 0.001f)
        {
            velocity.x = LMath::MoveTowards(
                velocity.x,
                targetVX,
                config.acceleration * control * dtSec);
        }
        else
        {
            velocity.x = LMath::MoveTowards(
                velocity.x,
                0.0f,
                config.deceleration * dtSec);
        }

        if (!grounded)
        {
            // Positive Y points down in this 2D demo.
            velocity.y += (baseGravity * config.gravityScale) * dtSec;
            if (velocity.y > config.maxFallSpeed)
                velocity.y = config.maxFallSpeed;
        }

        owner->transform.Position.x += velocity.x * dtSec;
        owner->transform.Position.y += velocity.y * dtSec;

        // Temporary ground resolution until collision system hookup.
        if (owner->transform.Position.y >= config.groundPlaneY)
        {
            owner->transform.Position.y = config.groundPlaneY;
            velocity.y = 0.0f;
            grounded = true;
        }
        else
        {
            grounded = false;
        }
    }

    void CharacterMovementComponent::Move(Lightning::V2 input)
    {
        SetMoveInput(input.x, input.y);
    }

    void CharacterMovementComponent::SetMoveInput(float x, float y)
    {
        // Keep API signature with Vector2 semantics used by scripts.
        moveInput.x = LMath::Clamp(x, -1.0f, 1.0f);
        moveInput.y = LMath::Clamp(y, -1.0f, 1.0f);
    }

    void CharacterMovementComponent::Jump()
    {
        if (!grounded) return;

        grounded = false;
        velocity.y = -config.jumpForce;
    }

    bool CharacterMovementComponent::IsGrounded() const
    {
        return grounded;
    }
}
