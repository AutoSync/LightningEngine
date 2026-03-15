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

		// World position = sum of all ancestor local positions (no rotation cascade yet).
		Lightning::V2 WorldPosition() const
		{
			Lightning::V2 pos = { transform.Position.x, transform.Position.y };
			if (parent)
			{
				Lightning::V2 p = parent->WorldPosition();
				pos.x += p.x;
				pos.y += p.y;
			}
			return pos;
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
