#pragma once

#include "ModeState.hpp"

namespace aidaw {

class AiMode {
public:
    void activate();
    void deactivate();
    bool isActive() const { return active; }

private:
    bool active = false;
};

}  // namespace aidaw
