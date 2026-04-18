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
#include <unordered_map>
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
			lookupDirty = true;
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
				lookupDirty = true;
				return owned;
			}
			return nullptr;
		}

		// Find first node with a matching name. Returns nullptr if not found.
		Node* FindNode(const std::string& name) const
		{
			EnsureLookup();
			auto it = nameIndex.find(name);
			return (it != nameIndex.end()) ? it->second : nullptr;
		}

		// Find first node with a matching tag. Returns nullptr if not found.
		Node* FindNodeByTag(const std::string& tag) const
		{
			EnsureLookup();
			auto it = tagIndex.find(tag);
			return (it != tagIndex.end()) ? it->second : nullptr;
		}

		std::vector<Node*> FindNodesByTag(const std::string& tag) const
		{
			EnsureLookup();
			std::vector<Node*> matches;
			auto range = tagIndex.equal_range(tag);
			for (auto it = range.first; it != range.second; ++it)
				matches.push_back(it->second);
			return matches;
		}

		void MarkNodeLookupDirty()
		{
			lookupDirty = true;
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
			nameIndex.clear();
			tagIndex.clear();
			lookupDirty = false;
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
		void EnsureLookup() const
		{
			if (!lookupDirty)
				return;

			nameIndex.clear();
			tagIndex.clear();
			for (const auto& n : nodes)
			{
				nameIndex[n->name] = n.get();
				if (!n->tag.empty())
					tagIndex.emplace(n->tag, n.get());
			}
			lookupDirty = false;
		}

		std::vector<std::unique_ptr<Node>> nodes;
		mutable std::unordered_map<std::string, Node*> nameIndex;
		mutable std::unordered_multimap<std::string, Node*> tagIndex;
		mutable bool lookupDirty = true;
	};

} // namespace LightningEngine
