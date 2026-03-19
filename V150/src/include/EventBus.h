// EventBus.h — Lightweight type-safe event system.
//
// Each EventBus<T> manages subscribers for a single event type T.
// Supports immediate dispatch (Emit) and deferred dispatch (Post + FlushQueue).
//
// Usage:
//   struct DamageEvent { Node* source; float amount; };
//
//   EventBus<DamageEvent> onDamage;
//
//   // Subscribe (returns a token for later unsubscribe):
//   EventToken tok = onDamage.Subscribe([](const DamageEvent& e) {
//       printf("took %.1f damage\n", e.amount);
//   });
//
//   // Emit immediately:
//   onDamage.Emit({ attacker, 25.f });
//
//   // Deferred:
//   onDamage.Post({ attacker, 10.f });
//   onDamage.FlushQueue();   // call once per frame
//
//   // Unsubscribe:
//   onDamage.Unsubscribe(tok);
#pragma once
#include <functional>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace LightningEngine {

// Opaque subscription handle returned by Subscribe().
using EventToken = uint32_t;

// ── EventBus<T> ──────────────────────────────────────────────────────────────

template<typename T>
class EventBus {
public:
    using Handler = std::function<void(const T&)>;

    // Subscribe a handler. Returns a token used to unsubscribe.
    EventToken Subscribe(Handler handler)
    {
        EventToken tok = nextToken++;
        subscribers[tok] = std::move(handler);
        return tok;
    }

    // Remove a previously subscribed handler by token.
    void Unsubscribe(EventToken token)
    {
        subscribers.erase(token);
    }

    // Immediately invoke all handlers with event.
    void Emit(const T& event) const
    {
        for (const auto& [tok, fn] : subscribers)
            if (fn) fn(event);
    }

    // Enqueue event for deferred dispatch via FlushQueue().
    void Post(const T& event)
    {
        queue.push_back(event);
    }

    // Dispatch all queued events (FIFO) and clear the queue.
    void FlushQueue()
    {
        // Copy first in case handlers post new events.
        std::vector<T> current = std::move(queue);
        queue.clear();
        for (const auto& ev : current)
            Emit(ev);
    }

    // Remove all subscribers and clear the deferred queue.
    void Clear()
    {
        subscribers.clear();
        queue.clear();
        nextToken = 0;
    }

    bool HasSubscribers() const { return !subscribers.empty(); }
    int  PendingCount()   const { return (int)queue.size(); }

private:
    std::unordered_map<EventToken, Handler> subscribers;
    std::vector<T>                          queue;
    EventToken                              nextToken = 0;
};

// ── Common engine event types ────────────────────────────────────────────────

// Forward declarations (include the relevant headers before using these).
class Node;

struct NodeAddedEvent   { Node* node; };
struct NodeRemovedEvent { Node* node; };
struct SceneLoadedEvent { std::string scenePath; };
struct SceneSavedEvent  { std::string scenePath; };
struct PlayEvent        {};   // editor entered play mode
struct StopEvent        {};   // editor stopped play mode

} // namespace LightningEngine
