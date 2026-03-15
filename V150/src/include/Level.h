// Level.h — Scene / phase manager. Owns a collection of Nodes.
// Receives Renderer and InputManager context from GameInstance via SetContext().
// Subclass and implement Initialize/Shutdown. Override Update/Render to add
// custom logic around the automatic node cascade.
//
// CLASS HIERARCHY:
// Window -> GameInstance -> [ LEVEL ] -> Node -> Component
//
// Usage (C++):
//   class MyLevel : public Level {
//   public:
//       void Initialize() override {
//           auto player = std::make_unique<Node>("Player");
//           player->AddComponent<SpriteRendererComponent>();
//           AddNode(std::move(player));
//       }
//       void Shutdown() override {}
//   };
//
//   // In GameInstance::Initialize():
//   myLevel.SetContext(renderer, inputManager);
//   myLevel.Initialize();

#pragma once
#include <vector>
#include <memory>
#include <string>
#include <algorithm>
#include "Node.h"

namespace LightningEngine
{
	class Level
	{
	protected:
		Renderer*            renderer = nullptr;
		InputManager*        input    = nullptr;

	public:
		virtual ~Level() = default;

		// -----------------------------------------------------------------------
		// Context — call from GameInstance::Initialize() before Level::Initialize()
		// -----------------------------------------------------------------------
		void SetContext(Renderer& r, InputManager& im)
		{
			renderer = &r;
			input    = &im;
		}

		// -----------------------------------------------------------------------
		// Node management
		// -----------------------------------------------------------------------

		// Transfer ownership of a node to this level. Injects context.
		void AddNode(std::unique_ptr<Node> node)
		{
			node->SetContext(renderer, input, this);
			nodes.push_back(std::move(node));
		}

		// Release a node from this level (returns ownership to caller).
		std::unique_ptr<Node> RemoveNode(Node* node)
		{
			auto it = std::find_if(nodes.begin(), nodes.end(),
				[node](const std::unique_ptr<Node>& n) { return n.get() == node; });
			if (it != nodes.end())
			{
				auto owned = std::move(*it);
				nodes.erase(it);
				return owned;
			}
			return nullptr;
		}

		// Find first node with a matching name. Returns nullptr if not found.
		Node* FindNode(const std::string& name) const
		{
			for (auto& n : nodes)
				if (n->name == name) return n.get();
			return nullptr;
		}

		// Find first node with a matching tag. Returns nullptr if not found.
		Node* FindNodeByTag(const std::string& tag) const
		{
			for (auto& n : nodes)
				if (n->tag == tag) return n.get();
			return nullptr;
		}

		// All nodes currently in this level.
		const std::vector<std::unique_ptr<Node>>& GetNodes() const { return nodes; }

		int NodeCount() const { return (int)nodes.size(); }

		// -----------------------------------------------------------------------
		// Lifecycle — Initialize and Shutdown are pure virtual.
		// Update and Render cascade to all active nodes by default.
		// Override them and call Level::Update(dt) / Level::Render() to extend.
		// -----------------------------------------------------------------------
		// Reset: calls Shutdown, clears all nodes, then calls Initialize again.
		void Reset()
		{
			Shutdown();
			nodes.clear();
			Initialize();
		}

		virtual void Initialize() = 0;
		virtual void Shutdown()   = 0;

		virtual void Update(float dt)
		{
			for (auto& n : nodes) n->Update(dt);
		}

		virtual void Render()
		{
			for (auto& n : nodes) n->Render();
		}

	private:
		std::vector<std::unique_ptr<Node>> nodes;
	};

} // namespace LightningEngine
