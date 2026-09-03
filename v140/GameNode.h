#pragma once

#include "Types.h"

#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace LightningEngine
{
    class GameNode;

    // Behaviour/render unit owned by a GameNode.
    class GameComponent
    {
        friend class GameNode;

    public:
        virtual ~GameComponent() = default;

        GameNode* GetOwner() const { return owner; }
        bool IsEnabled() const { return enabled; }
        bool IsActiveAndEnabled() const { return enabledInHierarchy; }
        void SetEnabled(bool value);

        virtual void OnAttach() {}
        virtual void OnStart() {}
        virtual void OnEnable() {}
        virtual void OnDisable() {}
        virtual void OnUpdate(float /*deltaTime*/) {}
        virtual void OnLateUpdate(float /*deltaTime*/) {}
        virtual void OnRender() {}
        virtual void OnDestroy() {}
        virtual void OnDetach() {}

    protected:
        GameNode* owner = nullptr;
        bool enabled = true;

    private:
        bool started = false;
        bool enabledInHierarchy = false;
        bool destroying = false;
    };

    // Scene/gameplay node. Ownership of children and components is explicit;
    // the node tree therefore has deterministic destruction and no raw owning
    // parent/child links.
    class GameNode
    {
    public:
        explicit GameNode(const char* nodeName = "GameNode")
            : name(nodeName ? nodeName : "GameNode") {}

        explicit GameNode(const std::string& nodeName)
            : name(nodeName) {}

        virtual ~GameNode() { DestroyRecursive(); }

        GameNode(const GameNode&) = delete;
        GameNode& operator=(const GameNode&) = delete;
        GameNode(GameNode&&) = delete;
        GameNode& operator=(GameNode&&) = delete;

        std::string name;
        std::string tag;
        Transform transform;

        bool IsActiveSelf() const { return activeSelf; }
        bool IsActiveInHierarchy() const { return activeInHierarchy; }

        void SetActive(bool value)
        {
            if (destroyed || destroying)
                return;
            if (activeSelf == value)
                return;
            activeSelf = value;
            RefreshActivation();
        }

        GameNode* GetParent() const { return parent; }

        // Transfers ownership to this node. The node must not already have a
        // parent and cycles are rejected.
        bool AddChild(std::unique_ptr<GameNode> child)
        {
            if (!child || child.get() == this || child->parent != nullptr ||
                child->destroyed || child->destroying || destroyed || destroying)
                return false;

            for (GameNode* ancestor = this; ancestor != nullptr; ancestor = ancestor->parent)
            {
                if (ancestor == child.get())
                    return false;
            }

            child->parent = this;
            GameNode* childPointer = child.get();
            children.push_back(std::move(child));

            if (started)
                childPointer->StartRecursive();
            childPointer->RefreshActivation();
            return true;
        }

        template <typename T, typename... Args>
        T* CreateChild(Args&&... args)
        {
            static_assert(std::is_base_of<GameNode, T>::value,
                "T must derive from LightningEngine::GameNode");
            auto child = std::make_unique<T>(std::forward<Args>(args)...);
            T* pointer = child.get();
            return AddChild(std::move(child)) ? pointer : nullptr;
        }

        // Detaches without destroying. The lifecycle state is preserved.
        std::unique_ptr<GameNode> RemoveChild(GameNode* child)
        {
            if (child == nullptr)
                return nullptr;

            auto iterator = std::find_if(children.begin(), children.end(),
                [child](const std::unique_ptr<GameNode>& candidate)
                {
                    return candidate.get() == child;
                });
            if (iterator == children.end())
                return nullptr;

            child->parent = nullptr;
            auto owned = std::move(*iterator);
            children.erase(iterator);
            owned->RefreshActivation();
            return owned;
        }

        void DestroyChild(GameNode* child)
        {
            auto owned = RemoveChild(child);
            if (owned)
                owned->DestroyRecursive();
        }

        const std::vector<std::unique_ptr<GameNode>>& GetChildren() const
        {
            return children;
        }

        template <typename T, typename... Args>
        T* AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of<GameComponent, T>::value,
                "T must derive from LightningEngine::GameComponent");

            if (destroyed || destroying)
                return nullptr;

            for (const auto& component : components)
            {
                if (auto* existing = dynamic_cast<T*>(component.get()))
                    return existing;
            }

            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T* pointer = component.get();
            pointer->owner = this;
            pointer->OnAttach();
            components.push_back(std::move(component));

            if (started)
            {
                StartComponent(*pointer);
                RefreshComponentActivation(*pointer);
            }
            return pointer;
        }

        template <typename T>
        T* GetComponent() const
        {
            for (const auto& component : components)
            {
                if (auto* result = dynamic_cast<T*>(component.get()))
                    return result;
            }
            return nullptr;
        }

        template <typename T>
        bool HasComponent() const { return GetComponent<T>() != nullptr; }

        template <typename T>
        bool RemoveComponent()
        {
            static_assert(std::is_base_of<GameComponent, T>::value,
                "T must derive from LightningEngine::GameComponent");
            auto iterator = std::find_if(components.begin(), components.end(),
                [](const std::unique_ptr<GameComponent>& component)
                {
                    return dynamic_cast<T*>(component.get()) != nullptr;
                });
            if (iterator == components.end())
                return false;

            DestroyComponent(**iterator);
            components.erase(iterator);
            return true;
        }

        const std::vector<std::unique_ptr<GameComponent>>& GetComponents() const
        {
            return components;
        }

        GameNode* Find(const std::string& nodeName)
        {
            if (name == nodeName)
                return this;
            for (const auto& child : children)
            {
                if (GameNode* result = child->Find(nodeName))
                    return result;
            }
            return nullptr;
        }

        const GameNode* Find(const std::string& nodeName) const
        {
            if (name == nodeName)
                return this;
            for (const auto& child : children)
            {
                if (const GameNode* result = child->Find(nodeName))
                    return result;
            }
            return nullptr;
        }

        glm::mat4 LocalMatrix() const
        {
            glm::mat4 matrix(1.0f);
            matrix = glm::translate(matrix, glm::vec3(transform.Position.x,
                                                       transform.Position.y,
                                                       transform.Position.z));
            matrix = glm::rotate(matrix, glm::radians(transform.Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
            matrix = glm::rotate(matrix, glm::radians(transform.Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
            matrix = glm::rotate(matrix, glm::radians(transform.Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
            matrix = glm::scale(matrix, glm::vec3(transform.Scale.x,
                                                   transform.Scale.y,
                                                   transform.Scale.z));
            return matrix;
        }

        glm::mat4 WorldMatrix() const
        {
            return parent ? parent->WorldMatrix() * LocalMatrix() : LocalMatrix();
        }

        V3 WorldPosition() const
        {
            const glm::mat4 matrix = WorldMatrix();
            return V3(matrix[3][0], matrix[3][1], matrix[3][2]);
        }

        void SetTransform(const Transform& value) { transform = value; }
        void SetPosition(const V3& value) { transform.Position = value; }
        void SetRotation(const V3& value) { transform.Rotation = value; }
        void SetScale(const V3& value) { transform.Scale = value; }
        Transform GetTransform() const { return transform; }
        V3 GetPosition() const { return transform.Position; }
        V3 GetRotation() const { return transform.Rotation; }
        V3 GetScale() const { return transform.Scale; }

        // Lifecycle: create/start/enable, update, late-update, render, then
        // disable/destroy/detach. Each phase cascades parent before children.
        void StartRecursive()
        {
            if (destroyed || destroying)
                return;

            if (!created)
            {
                created = true;
                OnCreate();
            }

            if (!started)
            {
                started = true;
                OnStart();
                for (const auto& component : components)
                    StartComponent(*component);
            }

            RefreshActivation();
            for (const auto& child : children)
                child->StartRecursive();
        }

        void UpdateRecursive(float deltaTime)
        {
            if (!activeInHierarchy || destroyed || destroying)
                return;
            OnUpdate(deltaTime);
            for (const auto& component : components)
                if (component->enabledInHierarchy)
                    component->OnUpdate(deltaTime);
            for (const auto& child : children)
                child->UpdateRecursive(deltaTime);
        }

        void LateUpdateRecursive(float deltaTime)
        {
            if (!activeInHierarchy || destroyed || destroying)
                return;
            OnLateUpdate(deltaTime);
            for (const auto& component : components)
                if (component->enabledInHierarchy)
                    component->OnLateUpdate(deltaTime);
            for (const auto& child : children)
                child->LateUpdateRecursive(deltaTime);
        }

        void RenderRecursive()
        {
            if (!activeInHierarchy || destroyed || destroying)
                return;
            OnRender();
            for (const auto& component : components)
                if (component->enabledInHierarchy)
                    component->OnRender();
            for (const auto& child : children)
                child->RenderRecursive();
        }

        void DestroyRecursive()
        {
            if (destroyed || destroying)
                return;
            destroying = true;

            if (activeInHierarchy)
            {
                activeInHierarchy = false;
                DisableComponentsAndNode();
            }

            for (auto& child : children)
                child->DestroyRecursive();
            children.clear();

            for (auto iterator = components.rbegin(); iterator != components.rend(); ++iterator)
                DestroyComponent(**iterator);
            components.clear();

            if (created)
                OnDestroy();

            parent = nullptr;
            destroyed = true;
            destroying = false;
        }

        bool IsStarted() const { return started; }
        bool IsDestroyed() const { return destroyed; }

        virtual void OnCreate() {}
        virtual void OnStart() {}
        virtual void OnEnable() {}
        virtual void OnDisable() {}
        virtual void OnUpdate(float /*deltaTime*/) {}
        virtual void OnLateUpdate(float /*deltaTime*/) {}
        virtual void OnRender() {}
        virtual void OnDestroy() {}

    private:
        bool IsEffectivelyActive() const
        {
            return activeSelf && (parent == nullptr || parent->activeInHierarchy);
        }

        void RefreshActivation()
        {
            const bool next = started && !destroyed && !destroying && IsEffectivelyActive();
            if (next != activeInHierarchy)
            {
                activeInHierarchy = next;
                if (next)
                {
                    OnEnable();
                    for (const auto& component : components)
                        RefreshComponentActivation(*component);
                }
                else
                {
                    DisableComponentsAndNode();
                }
            }
            for (const auto& child : children)
                child->RefreshActivation();
        }

        void RefreshComponentActivation(GameComponent& component)
        {
            const bool next = component.started && component.enabled && activeInHierarchy;
            if (next == component.enabledInHierarchy)
                return;
            component.enabledInHierarchy = next;
            if (next)
                component.OnEnable();
            else
                component.OnDisable();
        }

        void DisableComponentsAndNode()
        {
            for (const auto& component : components)
                RefreshComponentActivation(*component);
            OnDisable();
        }

        void StartComponent(GameComponent& component)
        {
            if (component.started || component.destroying)
                return;
            component.started = true;
            component.OnStart();
        }

        void DestroyComponent(GameComponent& component)
        {
            if (component.destroying)
                return;
            component.destroying = true;
            RefreshComponentActivation(component);
            if (component.started)
                component.OnDestroy();
            component.OnDetach();
            component.owner = nullptr;
            component.destroying = false;
        }

        friend class GameComponent;
        GameNode* parent = nullptr;
        std::vector<std::unique_ptr<GameNode>> children;
        std::vector<std::unique_ptr<GameComponent>> components;
        bool activeSelf = true;
        bool activeInHierarchy = false;
        bool created = false;
        bool started = false;
        bool destroying = false;
        bool destroyed = false;
    };

    using GameObject = GameNode;

    inline void GameComponent::SetEnabled(bool value)
    {
        if (enabled == value)
            return;
        enabled = value;
        if (owner != nullptr)
            owner->RefreshComponentActivation(*this);
    }
}
