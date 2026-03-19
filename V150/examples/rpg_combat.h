// RPG Combat 2D — Top-down 8-directional test bed.
//
// Tests each session:
//   [v0.15.0] Node, Level, BoxRendererComponent, RigidBody2DComponent,
//             CameraComponent, AnimationComponent (stubs ready for sprites),
//             custom components (Health, PlayerController 8-dir, EnemyAI),
//             AABB wall collision, HUD, InputManager
//
// Controls:
//   WASD / Arrows — 8-directional movement
//   F / Z         — melee attack
//   R             — reset level
//   ESC           — quit
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
#include "../src/include/components/Camera.h"
#include "../src/include/components/Animation.h"

using namespace LightningEngine;
using namespace Lightning;

// ============================================================
// HealthComponent — HP, damage, alive flag, hit-flash.
// ============================================================
class HealthComponent : public Component
{
public:
	int   hp       = 100;
	int   maxHp    = 100;
	bool  alive    = true;
	float hitFlash = 0.f;  // white-flash timer (seconds)

	void TakeDamage(int dmg)
	{
		if (!alive) return;
		hp -= dmg;
		hitFlash = 0.15f;
		if (hp <= 0) { hp = 0; alive = false; }
	}

	void Heal(int amount)
	{
		hp += amount;
		if (hp > maxHp) hp = maxHp;
	}

	float HealthPercent() const { return (float)hp / (float)maxHp; }

	void Update(float dt) override
	{
		if (hitFlash > 0.f) hitFlash -= dt / 1000.f;
	}
};

// ============================================================
// PlayerControllerComponent — Top-down 8-directional WASD.
// ============================================================
class PlayerControllerComponent : public Component
{
public:
	float speed          = 200.f;
	int   attackDmg      = 25;
	float attackRange    = 60.f;   // radius around player centre
	float attackCooldown = 0.4f;
	float attackTimer    = 0.f;
	bool  attacking      = false;
	float attackDuration = 0.15f;
	float attackActive   = 0.f;   // remaining window time

	// Face direction (for attack box & animation)
	V2 facing = { 1.f, 0.f };

	void Update(float dt) override
	{
		if (!owner || !owner->input) return;
		auto* rb = owner->GetComponent<RigidBody2DComponent>();
		if (!rb) return;

		float dtSec = dt / 1000.f;

		// --- 8-directional input ---
		float dx = 0.f, dy = 0.f;
		if (owner->input->IsKeyDown(SDL_SCANCODE_A) || owner->input->IsKeyDown(SDL_SCANCODE_LEFT))  dx -= 1.f;
		if (owner->input->IsKeyDown(SDL_SCANCODE_D) || owner->input->IsKeyDown(SDL_SCANCODE_RIGHT)) dx += 1.f;
		if (owner->input->IsKeyDown(SDL_SCANCODE_W) || owner->input->IsKeyDown(SDL_SCANCODE_UP))    dy -= 1.f;
		if (owner->input->IsKeyDown(SDL_SCANCODE_S) || owner->input->IsKeyDown(SDL_SCANCODE_DOWN))  dy += 1.f;

		// Normalize diagonal so speed is consistent in all 8 dirs
		V2 dir = { dx, dy };
		float len = LMath::Length(dir);
		if (len > 0.f)
		{
			dir = dir * (1.f / len);
			facing = dir;
		}

		rb->body.velocity = dir * speed;

		// --- Attack ---
		if (attackTimer > 0.f)
		{
			attackTimer -= dtSec;
			if (attackTimer <= 0.f) attackTimer = 0.f;
		}
		if (attackActive > 0.f)
		{
			attackActive -= dtSec;
			if (attackActive <= 0.f) { attackActive = 0.f; attacking = false; }
		}

		bool attackKey = owner->input->IsKeyPressed(SDL_SCANCODE_F)
		              || owner->input->IsKeyPressed(SDL_SCANCODE_Z);
		if (attackKey && attackTimer <= 0.f)
		{
			attackTimer  = attackCooldown;
			attackActive = attackDuration;
			attacking    = true;
		}
	}
};

// ============================================================
// EnemyAIComponent — Top-down chase + melee on contact.
// ============================================================
class EnemyAIComponent : public Component
{
public:
	Node* target         = nullptr;
	float speed          = 80.f;
	float detectionRange = 350.f;
	float attackRange    = 32.f;
	int   attackDmg      = 10;
	float attackTimer    = 0.f;
	float attackCooldown = 1.2f;

	void Update(float dt) override
	{
		if (!owner || !target) return;
		auto* rb     = owner->GetComponent<RigidBody2DComponent>();
		auto* health = owner->GetComponent<HealthComponent>();
		if (!rb || !health || !health->alive) return;

		float dtSec  = dt / 1000.f;
		V2 myPos     = { owner->transform.Position.x, owner->transform.Position.y };
		V2 targetPos = { target->transform.Position.x, target->transform.Position.y };
		float dist   = LMath::Distance(myPos, targetPos);

		if (dist < detectionRange && dist > 1.f)
		{
			V2 dir = LMath::Normalize(targetPos - myPos);
			rb->body.velocity = dir * speed;
		}
		else
		{
			rb->body.velocity = { 0.f, 0.f };
		}

		// Melee
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
// RPGLevel — top-down arena with walls and camera follow.
// ============================================================
class RPGLevel : public Level
{
private:
	Node* player   = nullptr;
	Node* camNode  = nullptr;
	int   score    = 0;
	bool  gameOver = false;

	// World size (pixels)
	static constexpr float kWorldW = 1600.f;
	static constexpr float kWorldH = 1200.f;
	static constexpr float kScreenW = 800.f;
	static constexpr float kScreenH = 600.f;

	// Entity half-extents for AABB collision
	static constexpr float kPlayerHW = 14.f;
	static constexpr float kPlayerHH = 14.f;
	static constexpr float kEnemyHW  = 13.f;
	static constexpr float kEnemyHH  = 13.f;

public:
	void Initialize() override
	{
		buildArena();
		spawnPlayer(200.f, 200.f);
		spawnEnemy("Goblin1",  700.f,  300.f);
		spawnEnemy("Goblin2", 1100.f,  500.f);
		spawnEnemy("Goblin3",  500.f,  900.f);
		spawnEnemy("Goblin4", 1300.f,  800.f);
		spawnCamera();
	}

	void Shutdown() override {}

	void Update(float dt) override
	{
		if (gameOver) return;
		Level::Update(dt);
		resolveWallCollisions();
		resolvePlayerAttack();
		cleanDeadEnemies();
	}

	void Render() override
	{
		Level::Render();
		renderHUD();
		if (gameOver) renderGameOver();
	}

private:
	// ----------------------------------------------------------
	// Build top-down arena: border walls + interior obstacles.
	// ----------------------------------------------------------
	void buildArena()
	{
		// Floor / background colour is set via renderer.SetClearColor in RPGGame.
		// Border walls (thick, surrounds the world)
		addWall("WallTop",    kWorldW * 0.5f,  15.f,       kWorldW, 30.f,  50, 50, 60);
		addWall("WallBottom", kWorldW * 0.5f,  kWorldH - 15.f, kWorldW, 30.f, 50, 50, 60);
		addWall("WallLeft",   15.f,            kWorldH * 0.5f, 30.f, kWorldH, 50, 50, 60);
		addWall("WallRight",  kWorldW - 15.f,  kWorldH * 0.5f, 30.f, kWorldH, 50, 50, 60);

		// Interior obstacles (rocks / pillars)
		addWall("Rock1",   400.f,  300.f, 80.f,  80.f,  80, 70, 55);
		addWall("Rock2",   900.f,  200.f, 60.f,  120.f, 80, 70, 55);
		addWall("Rock3",  1200.f,  600.f, 100.f, 60.f,  80, 70, 55);
		addWall("Rock4",   600.f,  800.f, 70.f,  70.f,  80, 70, 55);
		addWall("Rock5",  1000.f,  950.f, 90.f,  50.f,  80, 70, 55);
		addWall("Rock6",   300.f,  700.f, 50.f,  90.f,  80, 70, 55);
	}

	void addWall(const char* name, float cx, float cy, float w, float h,
	             Uint8 r, Uint8 g, Uint8 b)
	{
		auto node = std::make_unique<Node>(name);
		node->tag  = "wall";
		// Position is centre (BoxRenderer draws from pivot 0.5,0.5 by default)
		node->transform.Position = V3(cx, cy, 0.f);
		auto* box = node->AddComponent<BoxRendererComponent>();
		box->SetSize(w, h);
		box->SetColor(r, g, b);
		AddNode(std::move(node));
	}

	// ----------------------------------------------------------
	void spawnPlayer(float x, float y)
	{
		auto node = std::make_unique<Node>("Player");
		node->tag  = "player";
		node->transform.Position = V3(x, y, 0.f);

		auto* box = node->AddComponent<BoxRendererComponent>();
		box->SetSize(kPlayerHW * 2.f, kPlayerHH * 2.f);
		box->SetColor(70, 140, 210);
		box->SetOutline(120, 180, 255);

		auto* rb = node->AddComponent<RigidBody2DComponent>();
		rb->body.useGravity = false;
		rb->body.drag       = 0.f;

		node->AddComponent<HealthComponent>()->maxHp = 100;
		node->GetComponent<HealthComponent>()->hp    = 100;

		// AnimationComponent stub — ready for sprites once textures are added.
		// Clips will be wired to a SpriteRendererComponent when textures arrive.
		// auto* anim = node->AddComponent<AnimationComponent>();
		// anim->AddClip("idle",  4, 4, 0, 0, 1, 1.f,  true);
		// anim->AddClip("walk",  4, 4, 0, 1, 4, 10.f, true);
		// anim->AddClip("attack",4, 4, 0, 2, 3, 18.f, false);
		// anim->Play("idle");

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
		box->SetSize(kEnemyHW * 2.f, kEnemyHH * 2.f);
		box->SetColor(200, 60, 60);
		box->SetOutline(255, 120, 120);

		auto* rb = node->AddComponent<RigidBody2DComponent>();
		rb->body.useGravity = false;
		rb->body.drag       = 0.f;

		auto* hp = node->AddComponent<HealthComponent>();
		hp->hp = hp->maxHp = 60;

		auto* ai = node->AddComponent<EnemyAIComponent>();
		ai->target = player;

		AddNode(std::move(node));
	}

	void spawnCamera()
	{
		auto node = std::make_unique<Node>("Camera");
		auto* cam = node->AddComponent<CameraComponent>();
		cam->target   = player;
		cam->smoothing = 0.08f;  // smooth follow
		cam->SetWorldBounds(kWorldW, kWorldH, kScreenW, kScreenH);

		camNode = node.get();
		AddNode(std::move(node));
	}

	// ----------------------------------------------------------
	// AABB wall collision — push entities out of walls.
	// ----------------------------------------------------------
	void resolveWallCollisions()
	{
		pushEntityOutOfWalls(player,  kPlayerHW, kPlayerHH);
		for (auto& n : GetNodes())
		{
			if (n->tag == "enemy")
				pushEntityOutOfWalls(n.get(), kEnemyHW, kEnemyHH);
		}
	}

	void pushEntityOutOfWalls(Node* entity, float hw, float hh)
	{
		if (!entity) return;
		auto* rb = entity->GetComponent<RigidBody2DComponent>();
		if (!rb) return;

		for (auto& n : GetNodes())
		{
			if (n->tag != "wall") continue;
			auto* box = n->GetComponent<BoxRendererComponent>();
			if (!box) continue;

			float wx = n->transform.Position.x;
			float wy = n->transform.Position.y;
			float whw = box->width  * 0.5f;
			float whh = box->height * 0.5f;

			float ex = entity->transform.Position.x;
			float ey = entity->transform.Position.y;

			float overlapX = (hw + whw) - std::abs(ex - wx);
			float overlapY = (hh + whh) - std::abs(ey - wy);

			if (overlapX > 0.f && overlapY > 0.f)
			{
				// Push along the axis of least penetration
				if (overlapX < overlapY)
				{
					float sign = (ex > wx) ? 1.f : -1.f;
					entity->transform.Position.x += sign * overlapX;
					rb->body.position.x = entity->transform.Position.x;
					rb->body.velocity.x = 0.f;
				}
				else
				{
					float sign = (ey > wy) ? 1.f : -1.f;
					entity->transform.Position.y += sign * overlapY;
					rb->body.position.y = entity->transform.Position.y;
					rb->body.velocity.y = 0.f;
				}
			}
		}
	}

	// ----------------------------------------------------------
	// Player melee attack — circular range centred on player.
	// ----------------------------------------------------------
	void resolvePlayerAttack()
	{
		if (!player) return;
		auto* ctrl = player->GetComponent<PlayerControllerComponent>();
		if (!ctrl || !ctrl->attacking) return;

		V2 pPos = { player->transform.Position.x, player->transform.Position.y };

		for (auto& n : GetNodes())
		{
			if (n->tag != "enemy") continue;
			auto* hp = n->GetComponent<HealthComponent>();
			if (!hp || !hp->alive) continue;
			if (hp->hitFlash > 0.f) continue; // already hit this window

			V2 ePos = { n->transform.Position.x, n->transform.Position.y };
			if (LMath::Distance(pPos, ePos) <= ctrl->attackRange)
			{
				hp->TakeDamage(ctrl->attackDmg);
				if (!hp->alive) score++;
			}
		}
	}

	// Remove dead enemies
	void cleanDeadEnemies()
	{
		std::vector<Node*> toRemove;
		for (auto& n : GetNodes())
		{
			if (n->tag != "enemy") continue;
			auto* hp = n->GetComponent<HealthComponent>();
			if (hp && !hp->alive) toRemove.push_back(n.get());
		}
		for (auto* n : toRemove) RemoveNode(n);
	}

	// ----------------------------------------------------------
	// HUD — screen-space (ignores camera offset).
	// ----------------------------------------------------------
	void renderHUD()
	{
		if (!renderer || !player) return;
		auto* hp = player->GetComponent<HealthComponent>();
		if (!hp) return;

		renderer->BeginScreenSpace();

		// Player HP bar background
		renderer->SetDrawColor(50, 15, 15);
		renderer->FillRect(10.f, 10.f, 164.f, 18.f);
		// HP fill
		float pct = hp->HealthPercent();
		Uint8 r = (Uint8)(255 * (1.f - pct));
		Uint8 g = (Uint8)(200 * pct);
		renderer->SetDrawColor(r, g, 30);
		renderer->FillRect(11.f, 11.f, 162.f * pct, 16.f);
		// HP outline
		renderer->SetDrawColor(200, 200, 200, 180);
		renderer->DrawRect(10.f, 10.f, 164.f, 18.f);

		// Attack flash indicator
		auto* ctrl = player->GetComponent<PlayerControllerComponent>();
		if (ctrl && ctrl->attacking)
		{
			renderer->SetDrawColor(255, 220, 60, 200);
			renderer->FillRect(182.f, 10.f, 22.f, 18.f);
		}

		// Score dots
		for (int i = 0; i < score && i < 24; i++)
		{
			renderer->SetDrawColor(255, 200, 60);
			renderer->FillRect(10.f + i * 13.f, 36.f, 10.f, 10.f);
		}

		renderer->EndScreenSpace();

		// Enemy HP bars (world-space, above enemy sprite — camera applies)
		for (auto& n : GetNodes())
		{
			if (n->tag != "enemy") continue;
			auto* ehp = n->GetComponent<HealthComponent>();
			auto* box = n->GetComponent<BoxRendererComponent>();
			if (!ehp || !box) continue;

			// Hit flash
			if (ehp->hitFlash > 0.f)
				box->SetColor(255, 255, 255);
			else
				box->SetColor(200, 60, 60);

			float ex = n->transform.Position.x - 20.f;
			float ey = n->transform.Position.y - kEnemyHH - 12.f;
			renderer->SetDrawColor(50, 15, 15);
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
		renderer->BeginScreenSpace();
		renderer->SetDrawColor(0, 0, 0, 170);
		renderer->FillRect(200.f, 220.f, 400.f, 120.f);
		renderer->SetDrawColor(220, 50, 50);
		renderer->FillRect(260.f, 255.f, 280.f, 18.f);
		renderer->FillRect(260.f, 285.f, 280.f, 18.f);
		renderer->FillRect(260.f, 315.f, 280.f, 18.f);
		for (int i = 0; i < score && i < 20; i++)
		{
			renderer->SetDrawColor(255, 200, 60);
			renderer->FillRect(260.f + i * 13.f, 295.f, 10.f, 10.f);
		}
		renderer->EndScreenSpace();
	}
};

// ============================================================
// RPGGame — GameInstance entry point.
// ============================================================
class RPGGame : public LightningEngine::GameInstance
{
private:
	RPGLevel level;

public:
	void Initialize() override
	{
		renderer.SetClearColor(38, 50, 35); // dark green ground
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
		if (inputManager.IsKeyPressed(SDL_SCANCODE_R)) level.Reset();
		level.Update(dt);
	}

	void Render() override
	{
		renderer.Clear();
		level.Render();
		renderer.Present();
	}
};
