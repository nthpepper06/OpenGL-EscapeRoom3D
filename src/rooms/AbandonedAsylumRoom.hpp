#pragma once

#include "rooms/BaseRoom.hpp"

class AbandonedAsylumRoom : public BaseRoom {
public:
    AbandonedAsylumRoom();
    void build(InteractionManager& interactionMgr) override;
};
