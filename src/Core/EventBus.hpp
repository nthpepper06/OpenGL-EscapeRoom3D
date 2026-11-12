#ifndef CORE_EVENTBUS_HPP
#define CORE_EVENTBUS_HPP

#include <functional>
#include <vector>
#include <unordered_map>
#include <typeindex>
#include <memory>

namespace Core {

class IEvent {
public:
    virtual ~IEvent() = default;
};

class EventBus {
private:
    using Handler = std::function<void(const IEvent&)>;
    std::unordered_map<std::type_index, std::vector<Handler>> subscribers;

    EventBus() = default;

public:
    static EventBus& instance() {
        static EventBus bus;
        return bus;
    }

    template<typename EventType>
    void subscribe(std::function<void(const EventType&)> handler) {
        std::type_index typeIdx(typeid(EventType));
        subscribers[typeIdx].push_back([handler](const IEvent& e) {
            handler(static_cast<const EventType&>(e));
        });
    }

    template<typename EventType>
    void publish(const EventType& event) {
        std::type_index typeIdx(typeid(EventType));
        auto it = subscribers.find(typeIdx);
        if (it != subscribers.end()) {
            for (const auto& handler : it->second) {
                handler(event);
            }
        }
    }

    void clear() {
        subscribers.clear();
    }
};

} // namespace Core

#endif // CORE_EVENTBUS_HPP
