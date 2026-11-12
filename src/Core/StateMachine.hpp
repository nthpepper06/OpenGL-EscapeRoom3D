#ifndef CORE_STATEMACHINE_HPP
#define CORE_STATEMACHINE_HPP

#include <memory>
#include <unordered_map>
#include <iostream>

namespace Core {

template <typename StateEnum>
class IState {
public:
    virtual ~IState() = default;
    virtual void onEnter() {}
    virtual void onUpdate(float dt) {}
    virtual void onExit() {}
};

template <typename StateEnum>
class StateMachine {
private:
    StateEnum currentStateEnum;
    bool hasCurrentState = false;
    std::unordered_map<StateEnum, std::shared_ptr<IState<StateEnum>>> states;

public:
    void registerState(StateEnum stateEnum, std::shared_ptr<IState<StateEnum>> state) {
        states[stateEnum] = state;
    }

    void changeState(StateEnum newStateEnum) {
        if (hasCurrentState && currentStateEnum == newStateEnum) return;

        if (hasCurrentState && states.find(currentStateEnum) != states.end()) {
            states[currentStateEnum]->onExit();
        }

        currentStateEnum = newStateEnum;
        hasCurrentState = true;

        if (states.find(currentStateEnum) != states.end()) {
            states[currentStateEnum]->onEnter();
        }
    }

    void update(float dt) {
        if (hasCurrentState && states.find(currentStateEnum) != states.end()) {
            states[currentStateEnum]->onUpdate(dt);
        }
    }

    StateEnum getCurrentState() const {
        return currentStateEnum;
    }
};

} // namespace Core

#endif // CORE_STATEMACHINE_HPP
