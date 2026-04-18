// EventBus.h — Lightweight type-safe event system.
// Thread-safe for Subscribe/Unsubscribe/Emit/Post/FlushQueue/Clear. Emit copies
// the current handlers before dispatch so callbacks can subscribe/unsubscribe
// without invalidating iteration.
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
#include <mutex>
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
        std::lock_guard<std::mutex> lock(mutex);
        EventToken tok = nextToken++;
        subscribers[tok] = std::move(handler);
        return tok;
    }

    // Remove a previously subscribed handler by token.
    void Unsubscribe(EventToken token)
    {
        std::lock_guard<std::mutex> lock(mutex);
        subscribers.erase(token);
    }

    // Immediately invoke all handlers with event.
    void Emit(const T& event) const
    {
        std::vector<Handler> handlers;
        {
            std::lock_guard<std::mutex> lock(mutex);
            handlers.reserve(subscribers.size());
            for (const auto& [tok, fn] : subscribers)
                handlers.push_back(fn);
        }

        for (const auto& fn : handlers)
            if (fn) fn(event);
    }

    // Enqueue event for deferred dispatch via FlushQueue().
    void Post(const T& event)
    {
        std::lock_guard<std::mutex> lock(mutex);
        queue.push_back(event);
    }

    // Dispatch all queued events (FIFO) and clear the queue.
    void FlushQueue()
    {
        std::vector<T> current;
        {
            std::lock_guard<std::mutex> lock(mutex);
            // Move first in case handlers post new events during dispatch.
            current = std::move(queue);
            queue.clear();
        }

        for (const auto& ev : current)
            Emit(ev);
    }

    // Remove all subscribers and clear the deferred queue.
    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        subscribers.clear();
        queue.clear();
        nextToken = 0;
    }

    bool HasSubscribers() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return !subscribers.empty();
    }

    int PendingCount() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return (int)queue.size();
    }

private:
    mutable std::mutex                     mutex;
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
