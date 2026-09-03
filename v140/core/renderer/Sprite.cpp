#include "../../Sprite.h"

#include "../../Image.h"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

namespace
{
    constexpr const char* SPRITE_VERTEX_SHADER = R"glsl(
#version 330 core
layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 UV;
uniform vec2 viewport;
out vec2 TexCoord;
void main()
{
    vec2 ndc = vec2((Position.x / viewport.x) * 2.0 - 1.0,
                    1.0 - (Position.y / viewport.y) * 2.0);
    gl_Position = vec4(ndc, 0.0, 1.0);
    TexCoord = UV;
}
)glsl";

    constexpr const char* SPRITE_FRAGMENT_SHADER = R"glsl(
#version 330 core
in vec2 TexCoord;
uniform sampler2D spriteTexture;
uniform vec3 tint;
out vec4 FragColor;
void main()
{
    FragColor = texture(spriteTexture, TexCoord) * vec4(tint, 1.0);
}
)glsl";
}

namespace LightningEngine
{
    void SpriteAnimation::SetFrameDuration(float seconds)
    {
        const float duration = std::max(0.001f, seconds);
        for (auto& frame : frames)
            frame.duration = duration;
    }

    void SpriteAnimation::AddFrame(SpriteFrame frame)
    {
        frame.duration = std::max(0.001f, frame.duration);
        frames.push_back(frame);
        if (frames.size() == 1)
            Reset();
    }

    void SpriteAnimation::AddGridFrames(int columns, int rows, float duration)
    {
        if (columns <= 0 || rows <= 0)
            return;

        const float frameWidth = 1.0f / static_cast<float>(columns);
        const float frameHeight = 1.0f / static_cast<float>(rows);
        for (int row = 0; row < rows; ++row)
        {
            for (int column = 0; column < columns; ++column)
            {
                const float u0 = column * frameWidth;
                const float u1 = u0 + frameWidth;
                const float v1 = 1.0f - row * frameHeight;
                const float v0 = v1 - frameHeight;
                AddFrame({ V2(u0, v0), V2(u1, v1), duration });
            }
        }
    }

    void SpriteAnimation::Reset()
    {
        currentFrame = 0;
        elapsed = 0.0f;
        finished = false;
    }

    void SpriteAnimation::Play(bool restart)
    {
        if (restart)
            Reset();
        playing = true;
        finished = false;
    }

    void SpriteAnimation::Pause()
    {
        playing = false;
    }

    void SpriteAnimation::Stop()
    {
        playing = false;
        Reset();
    }

    void SpriteAnimation::Update(float deltaTime)
    {
        if (!playing || frames.empty() || deltaTime <= 0.0f)
            return;

        elapsed += deltaTime;
        while (elapsed >= frames[currentFrame].duration)
        {
            elapsed -= frames[currentFrame].duration;
            if (currentFrame + 1 < frames.size())
            {
                ++currentFrame;
                continue;
            }

            if (loop)
            {
                currentFrame = 0;
            }
            else
            {
                currentFrame = frames.size() - 1;
                elapsed = 0.0f;
                playing = false;
                finished = true;
                break;
            }
        }
    }

    const SpriteFrame* SpriteAnimation::GetCurrentFrameData() const
    {
        if (frames.empty() || currentFrame >= frames.size())
            return nullptr;
        return &frames[currentFrame];
    }

    SpriteRenderer::~SpriteRenderer()
    {
        Shutdown();
    }

    bool SpriteRenderer::Initialize()
    {
        if (initialized)
            return true;
        if (glfwGetCurrentContext() == nullptr)
            return false;

        shader = std::make_unique<Shader>(ShaderSource(SPRITE_VERTEX_SHADER, SPRITE_FRAGMENT_SHADER));
        shader->Init();
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);
        initialized = true;
        return true;
    }

    void SpriteRenderer::Shutdown()
    {
        if (glfwGetCurrentContext() != nullptr)
        {
            if (shader && shader->id != 0)
                glDeleteProgram(shader->id);
            if (VAO != 0)
                glDeleteVertexArrays(1, &VAO);
            if (VBO != 0)
                glDeleteBuffers(1, &VBO);
            if (EBO != 0)
                glDeleteBuffers(1, &EBO);
        }
        shader.reset();
        VAO = VBO = EBO = 0;
        initialized = false;
    }

    void SpriteRenderer::Draw(unsigned int textureId, V3 worldPosition, V2 size,
                               V2 pivot, SpriteFrame frame, C3 tint, V2 viewport)
    {
        if (!initialized || !shader || textureId == 0)
            return;

        const float left = worldPosition.x - size.x * pivot.x;
        const float top = worldPosition.y - size.y * pivot.y;
        const float right = left + size.x;
        const float bottom = top + size.y;
        const float vertices[] = {
            left,  top,    frame.uvMin.x, frame.uvMax.y,
            right, top,    frame.uvMax.x, frame.uvMax.y,
            right, bottom, frame.uvMax.x, frame.uvMin.y,
            left,  bottom, frame.uvMin.x, frame.uvMin.y
        };
        const unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        const GLboolean depthTestEnabled = glIsEnabled(GL_DEPTH_TEST);
        glDisable(GL_DEPTH_TEST);

        shader->Init();
        shader->SetV2("viewport", viewport);
        shader->SetV3("tint", V3(tint.r, tint.g, tint.b));
        shader->SetInt("spriteTexture", 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureId);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);

        if (depthTestEnabled)
            glEnable(GL_DEPTH_TEST);
        else
            glDisable(GL_DEPTH_TEST);
    }

    void SpriteNode::SetTexture(const std::string& path)
    {
        texturePath = path;
        if (IsStarted())
        {
            ReleaseTexture();
            LoadTexture();
        }
    }

    void SpriteNode::OnStart()
    {
        renderer = std::make_unique<SpriteRenderer>();
        renderer->Initialize();
        LoadTexture();
    }

    void SpriteNode::LoadTexture()
    {
        if (texturePath.empty() || textureId != 0 || glfwGetCurrentContext() == nullptr)
            return;

        Image image;
        textureId = image.TextureFromFile(texturePath.c_str());
    }

    void SpriteNode::ReleaseTexture()
    {
        if (textureId != 0 && glfwGetCurrentContext() != nullptr)
            glDeleteTextures(1, &textureId);
        textureId = 0;
    }

    SpriteFrame SpriteNode::GetCurrentSpriteFrame() const
    {
        return { uvMin, uvMax, 0.0f };
    }

    void SpriteNode::OnRender()
    {
        if (!visible || textureId == 0 || !renderer || !renderer->IsInitialized())
            return;
        renderer->Draw(textureId, WorldPosition(), size, pivot,
                       GetCurrentSpriteFrame(), tint, viewport);
    }

    void SpriteNode::OnDestroy()
    {
        ReleaseTexture();
        renderer.reset();
    }

    void AnimatedSpriteNode::AddAnimation(SpriteAnimation animation)
    {
        const std::string animationName = animation.GetName();
        animations[animationName] = std::move(animation);
        if (activeAnimation.empty())
            activeAnimation = animationName;
    }

    bool AnimatedSpriteNode::HasAnimation(const std::string& animationName) const
    {
        return animations.find(animationName) != animations.end();
    }

    bool AnimatedSpriteNode::PlayAnimation(const std::string& animationName, bool restart)
    {
        auto iterator = animations.find(animationName);
        if (iterator == animations.end())
            return false;
        activeAnimation = animationName;
        iterator->second.Play(restart);
        return true;
    }

    void AnimatedSpriteNode::PauseAnimation()
    {
        if (auto* animation = GetAnimation(activeAnimation))
            animation->Pause();
    }

    void AnimatedSpriteNode::StopAnimation()
    {
        if (auto* animation = GetAnimation(activeAnimation))
            animation->Stop();
    }

    void AnimatedSpriteNode::SetAnimationLoop(bool loop)
    {
        if (auto* animation = GetAnimation(activeAnimation))
            animation->SetLoop(loop);
    }

    const SpriteAnimation* AnimatedSpriteNode::GetAnimation(const std::string& animationName) const
    {
        auto iterator = animations.find(animationName);
        return iterator == animations.end() ? nullptr : &iterator->second;
    }

    SpriteAnimation* AnimatedSpriteNode::GetAnimation(const std::string& animationName)
    {
        auto iterator = animations.find(animationName);
        return iterator == animations.end() ? nullptr : &iterator->second;
    }

    bool AnimatedSpriteNode::SetSpriteSheet(const std::string& path,
                                             const std::string& animationName,
                                             int columns, int rows,
                                             float frameDuration,
                                             bool playImmediately)
    {
        if (columns <= 0 || rows <= 0)
            return false;

        SetTexture(path);
        SpriteAnimation animation(animationName);
        animation.AddGridFrames(columns, rows, frameDuration);
        AddAnimation(std::move(animation));
        if (playImmediately)
            return PlayAnimation(animationName, true);
        return true;
    }

    void AnimatedSpriteNode::OnUpdate(float deltaTime)
    {
        if (auto* animation = GetAnimation(activeAnimation))
            animation->Update(deltaTime);
    }

    SpriteFrame AnimatedSpriteNode::GetCurrentSpriteFrame() const
    {
        if (const auto* animation = GetAnimation(activeAnimation))
        {
            if (const auto* frame = animation->GetCurrentFrameData())
                return *frame;
        }
        return SpriteNode::GetCurrentSpriteFrame();
    }
}
