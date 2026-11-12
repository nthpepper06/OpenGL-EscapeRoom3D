#pragma once

#include "rooms/BaseRoom.hpp"

class MirrorRoom : public BaseRoom {
public:
    MirrorRoom();
    void build(InteractionManager& interactionMgr) override;
    std::string getObjectiveHint(const InteractionManager& im) const override;
};
