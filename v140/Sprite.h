#pragma once

#include "GameNode.h"
#include "Shader.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace LightningEngine
{
    struct SpriteFrame
    {
        // UV coordinates use the OpenGL convention: (0, 0) is the lower-left.
        V2 uvMin = V2(0.0f, 0.0f);
        V2 uvMax = V2(1.0f, 1.0f);
        float duration = 0.1f;
    };

    class SpriteAnimation
    {
    public:
        explicit SpriteAnimation(std::string animationName = "default")
            : name(std::move(animationName)) {}

        const std::string& GetName() const { return name; }
        const std::vector<SpriteFrame>& GetFrames() const { return frames; }
        std::vector<SpriteFrame>& GetFrames() { return frames; }
        std::size_t GetCurrentFrame() const { return currentFrame; }
        bool IsPlaying() const { return playing; }
        bool IsFinished() const { return finished; }
        bool IsLooping() const { return loop; }

        void SetLoop(bool value) { loop = value; }
        void SetFrameDuration(float seconds);
        void AddFrame(SpriteFrame frame);

        // Creates frames in row-major order. The V coordinate is inverted so
        // the first row corresponds to the top row of a conventional image.
        void AddGridFrames(int columns, int rows, float duration = 0.1f);

        void Reset();
        void Play(bool restart = false);
        void Pause();
        void Stop();
        void Update(float deltaTime);

        const SpriteFrame* GetCurrentFrameData() const;

    private:
        std::string name;
        std::vector<SpriteFrame> frames;
        std::size_t currentFrame = 0;
        float elapsed = 0.0f;
        bool loop = true;
        bool playing = false;
        bool finished = false;
    };

    class SpriteRenderer
    {
    public:
        SpriteRenderer() = default;
        ~SpriteRenderer();

        bool Initialize();
        void Shutdown();
        bool IsInitialized() const { return initialized; }
        void Draw(unsigned int textureId, V3 worldPosition, V2 size, V2 pivot,
                  SpriteFrame frame, C3 tint, V2 viewport);

    private:
        std::unique_ptr<Shader> shader;
        unsigned int VAO = 0;
        unsigned int VBO = 0;
        unsigned int EBO = 0;
        bool initialized = false;
    };

    class SpriteNode : public GameNode
    {
    public:
        explicit SpriteNode(const char* nodeName = "Sprite") : GameNode(nodeName) {}
        explicit SpriteNode(const std::string& nodeName) : GameNode(nodeName) {}
        ~SpriteNode() override = default;

        void SetTexture(const std::string& path);
        const std::string& GetTexturePath() const { return texturePath; }
        unsigned int GetTextureId() const { return textureId; }

        void SetSize(float width, float height) { size = V2(width, height); }
        void SetSize(V2 value) { size = value; }
        V2 GetSize() const { return size; }

        void SetPivot(V2 value) { pivot = value; }
        V2 GetPivot() const { return pivot; }
        void SetTint(C3 value) { tint = value; }
        C3 GetTint() const { return tint; }
        void SetViewport(float width, float height) { viewport = V2(width, height); }
        V2 GetViewport() const { return viewport; }
        void SetVisible(bool value) { visible = value; }
        bool IsVisible() const { return visible; }

        // Replaces the displayed portion of the texture for static sprites.
        void SetUV(V2 min, V2 max) { uvMin = min; uvMax = max; }

    protected:
        void OnStart() override;
        void OnRender() override;
        void OnDestroy() override;

        void LoadTexture();
        void ReleaseTexture();
        virtual SpriteFrame GetCurrentSpriteFrame() const;

        std::string texturePath;
        unsigned int textureId = 0;
        V2 size = V2(64.0f, 64.0f);
        V2 pivot = V2(0.5f, 0.5f);
        V2 viewport = V2(840.0f, 480.0f);
        V2 uvMin = V2(0.0f, 0.0f);
        V2 uvMax = V2(1.0f, 1.0f);
        C3 tint = C3(255, 255, 255);
        bool visible = true;
        std::unique_ptr<SpriteRenderer> renderer;
    };

    class AnimatedSpriteNode : public SpriteNode
    {
    public:
        explicit AnimatedSpriteNode(const char* nodeName = "AnimatedSprite")
            : SpriteNode(nodeName) {}
        explicit AnimatedSpriteNode(const std::string& nodeName)
            : SpriteNode(nodeName) {}
        ~AnimatedSpriteNode() override = default;

        void AddAnimation(SpriteAnimation animation);
        bool HasAnimation(const std::string& animationName) const;
        bool PlayAnimation(const std::string& animationName, bool restart = true);
        void PauseAnimation();
        void StopAnimation();
        void SetAnimationLoop(bool loop);
        const std::string& GetAnimationName() const { return activeAnimation; }
        const SpriteAnimation* GetAnimation(const std::string& animationName) const;
        SpriteAnimation* GetAnimation(const std::string& animationName);

        // Convenience for a conventional sprite sheet.
        bool SetSpriteSheet(const std::string& path, const std::string& animationName,
                            int columns, int rows, float frameDuration = 0.1f,
                            bool playImmediately = true);

    protected:
        void OnUpdate(float deltaTime) override;
        SpriteFrame GetCurrentSpriteFrame() const override;

    private:
        std::unordered_map<std::string, SpriteAnimation> animations;
        std::string activeAnimation;
    };

    using Sprite = SpriteNode;
    using SpriteAnimationNode = AnimatedSpriteNode;
}
