#ifndef DEBUG_DEBUGMENU_HPP
#define DEBUG_DEBUGMENU_HPP

#include <iostream>

namespace Debug {

class DebugMenu {
public:
    bool isVisible = false;
    bool godMode = false;
    bool infiniteAmmo = false;
    bool showPerformanceStats = true;

    void toggle() {
        isVisible = !isVisible;
    }
};

} // namespace Debug

#endif // DEBUG_DEBUGMENU_HPP
