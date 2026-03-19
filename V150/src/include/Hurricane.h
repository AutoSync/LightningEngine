// Hurricane.h — Particle system for visual effects (fire, smoke, explosions, etc.)
//
// Usage:
//   ParticleSystem ps;
//   ps.config.emitRate  = 80.f;      // particles per second
//   ps.config.lifeMin   = 0.5f;
//   ps.config.lifeMax   = 1.5f;
//   ps.config.startSize = 10.f;
//   ps.config.endSize   = 0.f;
//   ps.config.startColor = { 1.f, 0.6f, 0.1f, 1.f };
//   ps.config.endColor   = { 1.f, 0.f,  0.f,  0.f };
//   ps.config.velMinY   = -150.f;
//   ps.config.velMaxY   = -80.f;
//   ps.emitX = node->WorldPosition().x;
//   ps.emitY = node->WorldPosition().y;
//   ps.Play();
//
//   // Every frame (dt in milliseconds, same as Component::Update):
//   ps.Update(dt / 1000.f);
//   ps.Draw(renderer);
//
// ParticleSystemComponent: attach to a Node for automatic update + draw.
//   node->AddComponent<ParticleSystemComponent>()->system.config = cfg;
//   node->GetComponent<ParticleSystemComponent>()->system.Play();
#pragma once
#include <vector>
#include <cstdlib>
#include <cmath>
#include <SDL3/SDL.h>
#include "Renderer.h"
#include "Component.h"

namespace LightningEngine {

// ── Configuration ─────────────────────────────────────────────────────────────

struct ParticleEmitterConfig {
    float emitRate     = 50.f;   // particles per second
    float lifeMin      = 0.5f;   // minimum lifetime in seconds
    float lifeMax      = 2.f;    // maximum lifetime in seconds
    float startSize    = 8.f;    // initial size in pixels (square)
    float endSize      = 0.f;    // final size at end of life
    float r0=1.f,g0=0.5f,b0=0.1f,a0=1.f;   // startColor  RGBA [0,1]
    float r1=1.f,g1=0.f, b1=0.f, a1=0.f;   // endColor    RGBA [0,1]
    float velMinX      = -30.f;  // horizontal velocity range (px/s)
    float velMaxX      =  30.f;
    float velMinY      =-120.f;  // vertical velocity range (px/s, negative = up)
    float velMaxY      = -60.f;
    float gravity      = 80.f;   // gravity acceleration (px/s², positive = down)
    int   maxParticles = 512;
    bool  looping      = true;
    bool  burst        = false;  // emit maxParticles all at once on Play()
    float burstRadius  = 0.f;    // random spawn offset radius (0 = exact emitX/Y)
};

// ── Particle ──────────────────────────────────────────────────────────────────

struct Particle {
    float x, y;
    float vx, vy;
    float life;     // remaining lifetime (seconds)
    float maxLife;  // initial lifetime (for lerp)
    bool  alive = false;
};

// ── ParticleSystem ────────────────────────────────────────────────────────────

class ParticleSystem {
public:
    ParticleEmitterConfig config;
    float  emitX    = 0.f;
    float  emitY    = 0.f;
    bool   playing  = false;

    ParticleSystem() { pool.resize(512); }

    void Play()
    {
        playing    = true;
        emitAccum  = 0.f;
        if (config.burst) {
            pool.resize((size_t)config.maxParticles);
            for (int i = 0; i < config.maxParticles; i++)
                spawnParticle();
        }
    }

    void Stop()  { playing = false; }
    void Pause() { playing = false; }

    void Reset()
    {
        for (auto& p : pool) p.alive = false;
        aliveCount = 0;
        emitAccum  = 0.f;
    }

    int GetActiveCount() const { return aliveCount; }

    // dt in SECONDS.
    void Update(float dt)
    {
        pool.resize((size_t)config.maxParticles);

        // Update alive particles.
        aliveCount = 0;
        for (auto& p : pool) {
            if (!p.alive) continue;
            p.life -= dt;
            if (p.life <= 0.f) { p.alive = false; continue; }
            p.vy += config.gravity * dt;
            p.x  += p.vx * dt;
            p.y  += p.vy * dt;
            aliveCount++;
        }

        // Emit new particles.
        if (playing && config.looping && !config.burst) {
            emitAccum += config.emitRate * dt;
            while (emitAccum >= 1.f) {
                spawnParticle();
                emitAccum -= 1.f;
            }
        }
    }

    void Draw(Renderer& r) const
    {
        for (const auto& p : pool) {
            if (!p.alive) continue;

            float t = 1.f - (p.life / p.maxLife);
            t = t < 0.f ? 0.f : (t > 1.f ? 1.f : t);

            float size = config.startSize + (config.endSize - config.startSize) * t;
            if (size < 0.5f) continue;

            float cr = config.r0 + (config.r1 - config.r0) * t;
            float cg = config.g0 + (config.g1 - config.g0) * t;
            float cb = config.b0 + (config.b1 - config.b0) * t;
            float ca = config.a0 + (config.a1 - config.a0) * t;

            r.SetDrawColor(
                (Uint8)(cr * 255.f),
                (Uint8)(cg * 255.f),
                (Uint8)(cb * 255.f),
                (Uint8)(ca * 255.f));
            r.FillCircle(p.x, p.y, size * 0.5f);
        }
    }

private:
    std::vector<Particle> pool;
    int   aliveCount = 0;
    float emitAccum  = 0.f;

    static float randF(float lo, float hi)
    {
        if (lo >= hi) return lo;
        return lo + (float)rand() / (float)RAND_MAX * (hi - lo);
    }

    void spawnParticle()
    {
        // Find a dead slot.
        for (auto& p : pool) {
            if (p.alive) continue;

            float life = randF(config.lifeMin, config.lifeMax);
            float angle = randF(0.f, 6.2831853f);
            float r     = randF(0.f, config.burstRadius);

            p.x       = emitX + r * cosf(angle);
            p.y       = emitY + r * sinf(angle);
            p.vx      = randF(config.velMinX, config.velMaxX);
            p.vy      = randF(config.velMinY, config.velMaxY);
            p.life    = life;
            p.maxLife = life;
            p.alive   = true;
            return;
        }
    }
};

// ── ParticleSystemComponent ────────────────────────────────────────────────────

class ParticleSystemComponent : public Component {
public:
    ParticleSystem system;

    // dt from Component::Update is in milliseconds — convert to seconds.
    void Update(float dt) override
    {
        if (!owner) return;
        auto wp    = owner->WorldPosition();
        system.emitX = wp.x;
        system.emitY = wp.y;
        system.Update(dt / 1000.f);
    }

    void Render() override
    {
        if (!owner || !owner->renderer) return;
        system.Draw(*owner->renderer);
    }
};

} // namespace LightningEngine
