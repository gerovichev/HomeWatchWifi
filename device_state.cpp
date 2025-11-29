#include "device_state.h"
#include "logger.h"

DeviceState& DeviceState::getInstance() {
    static DeviceState instance;
    return instance;
}


