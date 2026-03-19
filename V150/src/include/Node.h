// Node.h — Scene entity. Base unit of the engine hierarchy.
// Owns a set of Components and a list of child Nodes.
// Context (Renderer, InputManager, Level) is injected by Level::AddNode.
//
// CLASS HIERARCHY:
// Window -> GameInstance -> Level -> [ NODE ] -> Component

#pragma once
#include <string>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Types.h"
#include "Component.h"

namespace LightningEngine
{
	class Renderer;      // forward — context pointer, no include needed
	class InputManager;  // forward
	class Level;         // forward

	class Node
	{
	public:
		std::string          name;
		std::string          tag;
		Lightning::Transform transform;
		bool                 active = true;

		// Context — injected by Level::AddNode. Read-only for components.
		Renderer*     renderer = nullptr;
		InputManager* input    = nullptr;
		Level*        level    = nullptr;

		// -----------------------------------------------------------------------
		// Construction
		// -----------------------------------------------------------------------
		explicit Node(const char* name = "Node") : name(name) {}
		explicit Node(const std::string& name)   : name(name) {}

		~Node()
		{
			for (auto& [type, comp] : components)
				comp->OnDetach();
			for (auto* child : children)
				child->parent = nullptr;
		}

		Node(const Node&)            = delete;
		Node& operator=(const Node&) = delete;
		Node(Node&&)                 = default;
		Node& operator=(Node&&)      = default;

		// -----------------------------------------------------------------------
		// Component system
		// -----------------------------------------------------------------------

		// Add a component of type T (one instance per type).
		// Returns the existing component if already attached.
		template<typename T>
		T* AddComponent()
		{
			static_assert(std::is_base_of<Component, T>::value,
				"T must derive from Component");

			auto key = std::type_index(typeid(T));
			auto it  = components.find(key);
			if (it != components.end())
				return static_cast<T*>(it->second.get());

			auto comp  = std::make_unique<T>();
			T*   ptr   = comp.get();
			ptr->owner = this;
			ptr->OnAttach();
			components.emplace(key, std::move(comp));
			return ptr;
		}

		template<typename T>
		T* GetComponent() const
		{
			auto it = components.find(std::type_index(typeid(T)));
			if (it != components.end())
				return static_cast<T*>(it->second.get());
			return nullptr;
		}

		template<typename T>
		bool HasComponent() const
		{
			return components.count(std::type_index(typeid(T))) > 0;
		}

		template<typename T>
		void RemoveComponent()
		{
			auto key = std::type_index(typeid(T));
			auto it  = components.find(key);
			if (it != components.end())
			{
				it->second->OnDetach();
				components.erase(it);
			}
		}

		// -----------------------------------------------------------------------
		// Hierarchy
		// -----------------------------------------------------------------------

		void AddChild(std::unique_ptr<Node> child)
		{
			child->parent   = this;
			child->renderer = renderer;
			child->input    = input;
			child->level    = level;
			children.push_back(child.get());
			ownedChildren.push_back(std::move(child));
		}

		std::unique_ptr<Node> RemoveChild(Node* child)
		{
			for (auto it = ownedChildren.begin(); it != ownedChildren.end(); ++it)
			{
				if (it->get() == child)
				{
					child->parent = nullptr;
					children.erase(std::find(children.begin(), children.end(), child));
					auto owned = std::move(*it);
					ownedChildren.erase(it);
					return owned;
				}
			}
			return nullptr;
		}

		Node*                        GetParent()   const { return parent;   }
		const std::vector<Node*>&    GetChildren() const { return children; }

		// -----------------------------------------------------------------------
		// Transform matrices (GLM, world-space cascade)
		// -----------------------------------------------------------------------

		// Local TRS matrix: translate → rotateY → rotateX → rotateZ → scale.
		glm::mat4 LocalMatrix() const
		{
			glm::mat4 m = glm::mat4(1.f);
			m = glm::translate(m, glm::vec3(transform.Position.x,
			                               transform.Position.y,
			                               transform.Position.z));
			m = glm::rotate(m, glm::radians(transform.Rotation.y), glm::vec3(0,1,0));
			m = glm::rotate(m, glm::radians(transform.Rotation.x), glm::vec3(1,0,0));
			m = glm::rotate(m, glm::radians(transform.Rotation.z), glm::vec3(0,0,1));
			m = glm::scale(m, glm::vec3(transform.Scale.x,
			                            transform.Scale.y,
			                            transform.Scale.z));
			return m;
		}

		// World matrix: cascades through the entire parent chain.
		glm::mat4 WorldMatrix() const
		{
			if (parent)
				return parent->WorldMatrix() * LocalMatrix();
			return LocalMatrix();
		}

		// 2D world position — derived from the full WorldMatrix (rotation-aware).
		Lightning::V2 WorldPosition() const
		{
			glm::mat4 wm = WorldMatrix();
			return { wm[3][0], wm[3][1] };
		}

		// 3D world position.
		Lightning::V3 WorldPosition3D() const
		{
			glm::mat4 wm = WorldMatrix();
			return { wm[3][0], wm[3][1], wm[3][2] };
		}

		// -----------------------------------------------------------------------
		// Cloning
		// -----------------------------------------------------------------------

		// Deep-copy this node (transform, tag, active state).
		// Override in subclasses to also copy components/children.
		virtual std::unique_ptr<Node> Clone() const
		{
			auto c = std::make_unique<Node>(name);
			c->tag       = tag;
			c->transform = transform;
			c->active    = active;
			return c;
		}

		// -----------------------------------------------------------------------
		// Lifecycle
		// -----------------------------------------------------------------------

		void Update(float dt)
		{
			if (!active) return;
			for (auto& [type, comp] : components)
				if (comp->enabled) comp->Update(dt);
			for (auto* child : children)
				child->Update(dt);
		}

		void Render()
		{
			if (!active) return;
			for (auto& [type, comp] : components)
				if (comp->enabled) comp->Render();
			for (auto* child : children)
				child->Render();
		}

		// Called by Level::AddNode — propagates context to this node and its children.
		void SetContext(Renderer* r, InputManager* im, Level* lv)
		{
			renderer = r;
			input    = im;
			level    = lv;
			for (auto& [type, comp] : components)
				comp->owner = this;
			for (auto* child : children)
				child->SetContext(r, im, lv);
		}

	private:
		std::unordered_map<std::type_index, std::unique_ptr<Component>> components;

		Node*                                parent = nullptr;
		std::vector<Node*>                   children;
		std::vector<std::unique_ptr<Node>>   ownedChildren;
	};

	// Alias for familiarity — both names are valid.
	using GameObject = Node;

} // namespace LightningEngine
