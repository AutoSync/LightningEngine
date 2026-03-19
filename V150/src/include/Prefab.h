// Prefab.h — Prototype-based node factory.
// Wraps a prototype instance of T; vends deep copies via Instantiate().
// T must derive from Node and implement Clone().
//
// Usage:
//   Prefab<Enemy> enemyPrefab;
//   enemyPrefab.Get().SetPosition(0, 0, 0);
//   auto enemy = enemyPrefab.Instantiate();  // unique_ptr<Enemy>
//   level.AddNode(std::move(enemy));
#pragma once
#include <memory>
#include "Node.h"

namespace LightningEngine {

template<typename T>
class Prefab {
    static_assert(std::is_base_of<Node, T>::value, "T must derive from Node");
public:
    Prefab() : proto(std::make_unique<T>()) {}

    // Access the prototype for one-time setup (position, tag, components…).
    T&       Get()       { return *proto; }
    const T& Get() const { return *proto; }

    // Deep-copy the prototype. Returns nullptr if Clone() returns wrong type.
    std::unique_ptr<T> Instantiate() const
    {
        auto cloned = proto->Clone();
        return std::unique_ptr<T>(static_cast<T*>(cloned.release()));
    }

private:
    std::unique_ptr<T> proto;
};

} // namespace LightningEngine
