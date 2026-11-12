#pragma once

#include "rooms/BaseRoom.hpp"

class OccultRitualRoom : public BaseRoom {
public:
    OccultRitualRoom();
    void build(InteractionManager& interactionMgr) override;
};
