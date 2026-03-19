// GameLevel.h — RPG2D example level.
// Demonstrates the full Level -> Node2D -> Component pipeline.
//
// Scene layout:
//   Player   (Node2D) — WASD movement, SpriteRendererComponent, RigidBody2DComponent
//   Enemy    (Node2D) — chases the player each frame, SpriteRendererComponent
//   Ground   (Node2D) — static platform drawn as a filled rect (BoxRenderer-style via SpriteRenderer)
//   HUDLabel (Node2D) — shows player position using Titan GUI (via external TitanUI ref)

#pragma once
#include "../../src/include/Level.h"
#include "../../src/include/Node2D.h"
#include "../../src/include/components/SpriteRenderer.h"
#include "../../src/include/components/RigidBody2D.h"
#include "../../src/include/Texture.h"

namespace RPG2D {

// -----------------------------------------------------------------------
// Player script — handles WASD input and updates physics body velocity
// -----------------------------------------------------------------------
class PlayerScript : public LightningEngine::Component
{
public:
    float speed = 200.f;

    void Update(float dt) override
    {
        if (!owner || !owner->input) return;

        auto* rb = owner->GetComponent<LightningEngine::RigidBody2DComponent>();
        if (!rb) return;

        Lightning::V2 dir = { 0.f, 0.f };

        if (owner->input->IsKeyDown(SDL_SCANCODE_A)) dir.x -= 1.f;
        if (owner->input->IsKeyDown(SDL_SCANCODE_D)) dir.x += 1.f;
        if (owner->input->IsKeyDown(SDL_SCANCODE_W)) dir.y -= 1.f;
        if (owner->input->IsKeyDown(SDL_SCANCODE_S)) dir.y += 1.f;

        rb->body.velocity.x = dir.x * speed;
        rb->body.velocity.y = dir.y * speed;

        // Jump on Space when on ground (simple Y threshold check)
        if (owner->input->IsKeyPressed(SDL_SCANCODE_SPACE) &&
            owner->transform.Position.y >= groundY - 2.f)
        {
            rb->body.ApplyImpulse({ 0.f, -450.f });
        }
    }

    float groundY = 500.f; // set by level after creation
};

// -----------------------------------------------------------------------
// Enemy chase script — moves toward the player each frame
// -----------------------------------------------------------------------
class EnemyScript : public LightningEngine::Component
{
public:
    LightningEngine::Node2D* target = nullptr;
    float speed = 80.f;

    void Update(float dt) override
    {
        if (!target || !owner) return;
        auto* self = static_cast<LightningEngine::Node2D*>(owner);
        self->MoveTo(target->WorldPosition(), speed, dt);
    }
};

// -----------------------------------------------------------------------
// GameLevel — the main scene
// -----------------------------------------------------------------------
class GameLevel : public LightningEngine::Level
{
public:
    // Pointers for external access (e.g., Editor or HUD)
    LightningEngine::Node2D* player = nullptr;
    LightningEngine::Node2D* enemy  = nullptr;

    // Textures — owned by the level, shared with SpriteRenderer components
    LightningEngine::Texture playerTex;
    LightningEngine::Texture enemyTex;

    void Initialize() override
    {
        const float groundY   = 500.f;
        const float groundH   = 32.f;
        const float screenW   = 1280.f;

        // --- Ground ---
        auto ground = std::make_unique<LightningEngine::Node2D>("Ground");
        ground->SetPosition(0.f, groundY);
        ground->tag = "ground";
        auto* gsr = ground->AddComponent<LightningEngine::SpriteRendererComponent>();
        gsr->SetSize(screenW, groundH);
        gsr->SetColor(80, 140, 60);  // green platform
        // No texture — solid colour rendered via tint on a 1x1 white texture
        // (requires a white pixel texture; left as nullptr here — add LoadWhite())
        AddNode(std::move(ground));

        // --- Player ---
        playerTex = renderer->LoadTexture("examples/rpg2D/assets/player_sprites.png");
        bool playerTexLoaded = playerTex.IsValid();

        auto playerNode = std::make_unique<LightningEngine::Node2D>("Player");
        playerNode->SetPosition(200.f, groundY - 64.f);
        playerNode->tag = "player";

        auto* psr = playerNode->AddComponent<LightningEngine::SpriteRendererComponent>();
        if (playerTexLoaded) {
            psr->texture = &playerTex;
            psr->SetSize(64.f, 64.f);
        } else {
            // Fallback — render as a cyan box
            psr->SetColor(0, 220, 220);
            psr->SetSize(32.f, 48.f);
        }

        auto* prb = playerNode->AddComponent<LightningEngine::RigidBody2DComponent>();
        prb->body.useGravity = true;
        prb->body.mass       = 1.f;
        prb->body.drag       = 0.05f;

        auto* ps = playerNode->AddComponent<PlayerScript>();
        ps->groundY = groundY;

        player = playerNode.get();
        AddNode(std::move(playerNode));

        // --- Enemy ---
        auto enemyNode = std::make_unique<LightningEngine::Node2D>("Enemy");
        enemyNode->SetPosition(900.f, groundY - 48.f);
        enemyNode->tag = "enemy";

        auto* esr = enemyNode->AddComponent<LightningEngine::SpriteRendererComponent>();
        esr->SetSize(32.f, 48.f);
        esr->SetColor(220, 60, 60);  // red enemy box

        auto* es = enemyNode->AddComponent<EnemyScript>();
        es->target = player;

        enemy = enemyNode.get();
        AddNode(std::move(enemyNode));
    }

    void Shutdown() override
    {
        player = nullptr;
        enemy  = nullptr;
        playerTex.Release();
        enemyTex.Release();
    }

    void Update(float dt) override
    {
        Level::Update(dt);  // cascade to all nodes

        // Clamp player to ground
        if (player) {
            auto* rb = player->GetComponent<LightningEngine::RigidBody2DComponent>();
            if (rb && player->GetPosition().y >= 500.f) {
                player->SetPosition(player->GetPosition().x, 500.f);
                rb->body.velocity.y = 0.f;
            }
        }
    }

    void Render() override
    {
        Level::Render();  // cascade to all nodes
    }
};

} // namespace RPG2D
