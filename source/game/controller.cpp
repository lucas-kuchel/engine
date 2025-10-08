#include <game/controller.hpp>
#include <game/physics.hpp>

namespace game {
    void halt(MovableBody& body, Controller& controller, app::WindowKeyReleasedEventInfo& eventInfo) {
        if (eventInfo.key == controller.leftBinding || eventInfo.key == controller.rightBinding) {
            body.acceleration.x = 0.0f;
        }
    }
}