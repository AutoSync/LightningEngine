// RPG Combat 2D — Live test bed for Node / Level / Component systems.
//
// Tests each session:
//   [v0.15.0] Node, Level, BoxRendererComponent, RigidBody2DComponent,
//             custom components (Health, PlayerController, EnemyAI),
//             platform collision, HUD (FillRect), InputManager
//
// Usage in main():
//   RPGGame game;
//   game.Run("RPG Combat 2D", 800, 600);

#pragma once
#include "../src/include/GameInstance.h"
#include "../src/include/Level.h"
#include "../src/include/Node.h"
#include "../src/include/Physics.h"
#include "../src/include/Math.h"
#include "../src/include/components/BoxRenderer.h"
#include "../src/include/components/RigidBody2D.h"

using namespace LightningEngine;
using namespace Lightning;

// ============================================================
// Custom components
// ============================================================

// HealthComponent — HP, damage, alive flag.
class HealthComponent : public Component
{
public:
	int  hp    = 100;
	int  maxHp = 100;
	bool alive = true;

	// Flash timer: renders in white briefly when hit.
	float hitFlash = 0.f;

	void TakeDamage(int dmg)
	{
		if (!alive) return;
		hp -= dmg;
		hitFlash = 0.15f;
		if (hp <= 0) { hp = 0; alive = false; }
	}

	void Heal(int amount)
	{
		hp = hp + amount;
		if (hp > maxHp) hp = maxHp;
	}

	float HealthPercent() const { return (float)hp / (float)maxHp; }

	void Update(float dt) override
	{
		if (hitFlash > 0.f) hitFlash -= dt / 1000.f;
	}
};

// PlayerControllerComponent — WASD/Arrow movement, Space to jump, F to attack.
class PlayerControllerComponent : public Component
{
public:
	float speed       = 220.f;
	float jumpForce   = 520.f;
	int   attackDmg   = 25;
	float attackRange = 70.f;  // horizontal reach in pixels
	float attackH     = 48.f;  // vertical range

	bool  onGround    = false;  // set by RPGLevel collision
	float attackTimer = 0.f;   // cooldown in seconds
	bool  attacking   = false; // true during the attack window
	float attackDuration = 0.12f;

	void Update(float dt) override
	{
		if (!owner || !owner->input) return;
		auto* rb = owner->GetComponent<RigidBody2DComponent>();
		if (!rb) return;

		float dtSec = dt / 1000.f;

		// Horizontal movement
		bool moveLeft  = owner->input->IsKeyDown(SDL_SCANCODE_A)
		              || owner->input->IsKeyDown(SDL_SCANCODE_LEFT);
		bool moveRight = owner->input->IsKeyDown(SDL_SCANCODE_D)
		              || owner->input->IsKeyDown(SDL_SCANCODE_RIGHT);

		if      (moveLeft)  rb->body.velocity.x = -speed;
		else if (moveRight) rb->body.velocity.x =  speed;
		else                rb->body.velocity.x *= 0.75f; // ground friction

		// Jump (only when on ground)
		if (onGround && (owner->input->IsKeyPressed(SDL_SCANCODE_SPACE)
		              || owner->input->IsKeyPressed(SDL_SCANCODE_W)
		              || owner->input->IsKeyPressed(SDL_SCANCODE_UP)))
		{
			rb->body.velocity.y = -jumpForce;
			onGround = false;
		}

		// Attack cooldown + window
		if (attackTimer > 0.f)
		{
			attackTimer -= dtSec;
			attacking = attackTimer > (attackTimer + attackDuration - attackDuration); // stays true
		}
		if (attackTimer <= 0.f) attacking = false;

		bool attackKey = owner->input->IsKeyPressed(SDL_SCANCODE_F)
		              || owner->input->IsKeyPressed(SDL_SCANCODE_Z);
		if (attackKey && attackTimer <= 0.f)
		{
			attackTimer = 0.4f; // total cooldown
			attacking   = true;
		}
	}
};

// EnemyAIComponent — chases the player, deals damage on contact.
class EnemyAIComponent : public Component
{
public:
	Node* target       = nullptr; // player node — set by Level after spawn
	float speed        = 90.f;
	float detectionRange = 350.f;
	float attackRange  = 36.f;
	int   attackDmg    = 10;
	float attackTimer  = 0.f;
	float attackCooldown = 1.2f;

	void Update(float dt) override
	{
		if (!owner || !target) return;
		auto* rb     = owner->GetComponent<RigidBody2DComponent>();
		auto* health = owner->GetComponent<HealthComponent>();
		if (!rb || !health || !health->alive) return;

		float dtSec = dt / 1000.f;

		V2 myPos     = { owner->transform.Position.x, owner->transform.Position.y };
		V2 targetPos = { target->transform.Position.x, target->transform.Position.y };
		float dist   = LMath::Distance(myPos, targetPos);

		// Chase if within detection range
		if (dist < detectionRange)
		{
			V2 dir = LMath::Normalize(targetPos - myPos);
			rb->body.velocity.x = dir.x * speed;
		}
		else
		{
			rb->body.velocity.x *= 0.6f;
		}

		// Attack on contact
		if (attackTimer > 0.f) attackTimer -= dtSec;

		if (dist < attackRange && attackTimer <= 0.f)
		{
			if (auto* tHealth = target->GetComponent<HealthComponent>())
			{
				tHealth->TakeDamage(attackDmg);
				attackTimer = attackCooldown;
			}
		}
	}
};

// ============================================================
// RPG Level
// ============================================================
class RPGLevel : public Level
{
private:
	Node* player  = nullptr;
	int   score   = 0;    // enemies defeated
	bool  gameOver = false;

	static constexpr float kPlayerHalfW = 18.f;
	static constexpr float kPlayerHalfH = 26.f;
	static constexpr float kEnemyHalfW  = 16.f;
	static constexpr float kEnemyHalfH  = 22.f;

public:
	void Initialize() override
	{
		buildLevel();
		spawnPlayer(200.f, 300.f);
		spawnEnemy("Enemy1", 550.f, 250.f);
		spawnEnemy("Enemy2", 680.f, 100.f);
	}

	void Shutdown() override {}

	void Update(float dt) override
	{
		if (gameOver) return;
		Level::Update(dt);        // cascade to all nodes
		resolveCollisions();
		resolvePlayerAttack();
		cleanDeadEnemies();
	}

	void Render() override
	{
		Level::Render();          // cascade to all nodes
		renderHUD();
		if (gameOver) renderGameOver();
	}

	// -------------------------------------------------------
private:
	// Build platforms / floor
	void buildLevel()
	{
		// Main floor
		addPlatform("Floor",    400.f, 570.f, 800.f, 40.f, 70, 70, 70);
		// Platforms
		addPlatform("Platform1", 180.f, 420.f, 160.f, 20.f, 90, 80, 60);
		addPlatform("Platform2", 450.f, 340.f, 200.f, 20.f, 90, 80, 60);
		addPlatform("Platform3", 660.f, 450.f, 130.f, 20.f, 90, 80, 60);
	}

	void addPlatform(const char* name, float x, float y, float w, float h,
	                 Uint8 r, Uint8 g, Uint8 b)
	{
		auto node = std::make_unique<Node>(name);
		node->tag  = "platform";
		node->transform.Position = V3(x, y, 0.f);
		auto* box = node->AddComponent<BoxRendererComponent>();
		box->SetSize(w, h);
		box->SetColor(r, g, b);
		AddNode(std::move(node));
	}

	void spawnPlayer(float x, float y)
	{
		auto node = std::make_unique<Node>("Player");
		node->tag  = "player";
		node->transform.Position = V3(x, y, 0.f);

		auto* box = node->AddComponent<BoxRendererComponent>();
		box->SetSize(kPlayerHalfW * 2.f, kPlayerHalfH * 2.f);
		box->SetColor(70, 130, 200); // blue

		auto* rb = node->AddComponent<RigidBody2DComponent>();
		rb->body.useGravity = true;
		rb->body.mass       = 1.f;
		rb->body.drag       = 0.02f;

		auto* hp = node->AddComponent<HealthComponent>();
		hp->hp = hp->maxHp = 100;

		node->AddComponent<PlayerControllerComponent>();

		player = node.get();
		AddNode(std::move(node));
	}

	void spawnEnemy(const char* name, float x, float y)
	{
		auto node = std::make_unique<Node>(name);
		node->tag  = "enemy";
		node->transform.Position = V3(x, y, 0.f);

		auto* box = node->AddComponent<BoxRendererComponent>();
		box->SetSize(kEnemyHalfW * 2.f, kEnemyHalfH * 2.f);
		box->SetColor(200, 60, 60); // red
		box->SetOutline(255, 120, 120);

		auto* rb = node->AddComponent<RigidBody2DComponent>();
		rb->body.useGravity = true;
		rb->body.mass       = 1.2f;
		rb->body.drag       = 0.05f;

		auto* hp = node->AddComponent<HealthComponent>();
		hp->hp = hp->maxHp = 60;

		auto* ai = node->AddComponent<EnemyAIComponent>();
		ai->target = player;

		AddNode(std::move(node));
	}

	// -------------------------------------------------------
	// Platform collision — simple AABB push-up for entities on platforms.
	void resolveCollisions()
	{
		resolveEntityOnPlatforms(player, kPlayerHalfW, kPlayerHalfH, true);

		for (auto& n : GetNodes())
		{
			if (n->tag == "enemy")
				resolveEntityOnPlatforms(n.get(), kEnemyHalfW, kEnemyHalfH, false);
		}
	}

	void resolveEntityOnPlatforms(Node* entity, float hw, float hh, bool isPlayer)
	{
		if (!entity) return;
		auto* rb = entity->GetComponent<RigidBody2DComponent>();
		if (!rb) return;

		float ex   = entity->transform.Position.x;
		float ey   = entity->transform.Position.y;
		float eBot = ey + hh;
		float eTop = ey - hh;
		float eL   = ex - hw;
		float eR   = ex + hw;

		if (isPlayer)
		{
			auto* ctrl = entity->GetComponent<PlayerControllerComponent>();
			if (ctrl) ctrl->onGround = false;
		}

		for (auto& n : GetNodes())
		{
			if (n->tag != "platform") continue;
			auto* box = n->GetComponent<BoxRendererComponent>();
			if (!box) continue;

			float px = n->transform.Position.x;
			float py = n->transform.Position.y;
			float pw = box->width  * 0.5f;
			float ph = box->height * 0.5f;

			float pL = px - pw, pR = px + pw;
			float pT = py - ph, pB = py + ph;

			bool xOverlap = eR > pL && eL < pR;
			bool wasAbove = (eBot - rb->body.velocity.y * 0.016f) <= pT + 2.f;

			if (xOverlap && wasAbove && eBot >= pT && eTop < pB)
			{
				entity->transform.Position.y = pT - hh;
				rb->body.position.y = entity->transform.Position.y;
				if (rb->body.velocity.y > 0.f) rb->body.velocity.y = 0.f;
				if (isPlayer)
				{
					auto* ctrl = entity->GetComponent<PlayerControllerComponent>();
					if (ctrl) ctrl->onGround = true;
				}
			}
		}

		// World bounds — clamp X, kill if falls off screen
		float& px = entity->transform.Position.x;
		px = LMath::Clamp(px, hw, 800.f - hw);
		rb->body.position.x = px;

		if (entity->transform.Position.y > 700.f)
		{
			if (isPlayer)
			{
				if (auto* hp = entity->GetComponent<HealthComponent>())
					hp->TakeDamage(hp->hp); // die on falling
			}
			else
			{
				if (auto* hp = entity->GetComponent<HealthComponent>())
					hp->alive = false;
			}
		}
	}

	// -------------------------------------------------------
	// Player melee attack: damages enemies within range.
	void resolvePlayerAttack()
	{
		if (!player) return;
		auto* ctrl = player->GetComponent<PlayerControllerComponent>();
		if (!ctrl || !ctrl->attacking) return;

		float px = player->transform.Position.x;
		float py = player->transform.Position.y;

		// Attack box: to the right for now (TODO: face direction)
		Rect2D attackBox = {
			px - ctrl->attackRange * 0.5f,
			py - ctrl->attackH * 0.5f,
			ctrl->attackRange,
			ctrl->attackH
		};

		for (auto& n : GetNodes())
		{
			if (n->tag != "enemy") continue;
			auto* hp  = n->GetComponent<HealthComponent>();
			auto* box = n->GetComponent<BoxRendererComponent>();
			if (!hp || !box || !hp->alive) continue;

			Rect2D enemyBox = box->GetBounds();
			if (attackBox.Overlaps(enemyBox) && hp->hitFlash <= 0.f)
			{
				hp->TakeDamage(ctrl->attackDmg);
				if (!hp->alive) score++;
			}
		}
	}

	// Remove dead enemies from the level.
	void cleanDeadEnemies()
	{
		std::vector<Node*> toRemove;
		for (auto& n : GetNodes())
		{
			if (n->tag != "enemy") continue;
			auto* hp = n->GetComponent<HealthComponent>();
			if (hp && !hp->alive)
				toRemove.push_back(n.get());
		}
		for (auto* n : toRemove)
			RemoveNode(n);
	}

	// -------------------------------------------------------
	// HUD — rendered with FillRect (no text yet, uses bar shapes).
	void renderHUD()
	{
		if (!renderer || !player) return;
		auto* hp = player->GetComponent<HealthComponent>();
		if (!hp) return;

		// Player HP bar — background
		renderer->SetDrawColor(60, 20, 20);
		renderer->FillRect(10.f, 10.f, 160.f, 16.f);

		// Player HP bar — fill
		float pct = hp->HealthPercent();
		Uint8 r = (Uint8)(255 * (1.f - pct));
		Uint8 g = (Uint8)(200 * pct);
		renderer->SetDrawColor(r, g, 30);
		renderer->FillRect(10.f, 10.f, 160.f * pct, 16.f);

		// HP bar outline
		renderer->SetDrawColor(200, 200, 200, 180);
		renderer->DrawRect(10.f, 10.f, 160.f, 16.f);

		// Attack flash indicator
		auto* ctrl = player->GetComponent<PlayerControllerComponent>();
		if (ctrl && ctrl->attacking)
		{
			renderer->SetDrawColor(255, 220, 60, 180);
			renderer->FillRect(180.f, 10.f, 20.f, 16.f);
		}

		// Score dots (one dot per kill, max 20)
		for (int i = 0; i < score && i < 20; i++)
		{
			renderer->SetDrawColor(255, 200, 60);
			renderer->FillRect(10.f + i * 14.f, 34.f, 10.f, 10.f);
		}

		// Enemy HP bars (above enemies)
		for (auto& n : GetNodes())
		{
			if (n->tag != "enemy") continue;
			auto* ehp = n->GetComponent<HealthComponent>();
			auto* box = n->GetComponent<BoxRendererComponent>();
			if (!ehp || !box) continue;

			float ex = n->transform.Position.x - 20.f;
			float ey = n->transform.Position.y - kEnemyHalfH - 12.f;

			// Flash white when hit
			if (ehp->hitFlash > 0.f)
			{
				auto* brc = n->GetComponent<BoxRendererComponent>();
				if (brc) brc->SetColor(255, 255, 255);
			}
			else
			{
				auto* brc = n->GetComponent<BoxRendererComponent>();
				if (brc) brc->SetColor(200, 60, 60);
			}

			renderer->SetDrawColor(60, 20, 20);
			renderer->FillRect(ex, ey, 40.f, 6.f);
			renderer->SetDrawColor(200, 60, 60);
			renderer->FillRect(ex, ey, 40.f * ehp->HealthPercent(), 6.f);
		}

		// Check game over
		if (auto* phh = player->GetComponent<HealthComponent>())
			if (!phh->alive) gameOver = true;
	}

	void renderGameOver()
	{
		if (!renderer) return;
		// Darkened overlay
		renderer->SetDrawColor(0, 0, 0, 160);
		renderer->FillRect(200.f, 220.f, 400.f, 120.f);
		// "GAME OVER" visual: two thick bars forming an X shape
		renderer->SetDrawColor(220, 50, 50);
		renderer->FillRect(260.f, 255.f, 280.f, 18.f);
		renderer->FillRect(260.f, 285.f, 280.f, 18.f);
		renderer->FillRect(260.f, 315.f, 280.f, 18.f);
		// Score bar
		for (int i = 0; i < score && i < 20; i++)
		{
			renderer->SetDrawColor(255, 200, 60);
			renderer->FillRect(260.f + i * 13.f, 295.f, 10.f, 10.f);
		}
	}
};

// ============================================================
// RPGGame — GameInstance entry point
// ============================================================
class RPGGame : public LightningEngine::GameInstance
{
private:
	RPGLevel level;

public:
	void Initialize() override
	{
		renderer.SetClearColor(28, 22, 38); // dark purple sky
		level.SetContext(renderer, inputManager);
		level.Initialize();
	}

	void Shutdown() override
	{
		level.Shutdown();
	}

	void Update(float dt) override
	{
		if (inputManager.IsKeyPressed(SDL_SCANCODE_ESCAPE)) Quit();
		if (inputManager.IsKeyPressed(SDL_SCANCODE_R))
		{
			level.Reset(); // Shutdown → clear nodes → Initialize
		}
		level.Update(dt);
	}

	void Render() override
	{
		renderer.Clear();
		level.Render();
		renderer.Present();
	}
};
