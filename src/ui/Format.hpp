#pragma once

#include <chrono>
#include <string>

namespace typr::ui {

inline std::string formatDuration(std::chrono::milliseconds ms) {
    auto sec = ms.count() / 1000;
    auto min = sec / 60;
    sec %= 60;
    if (min > 0) return std::to_string(min) + "m" + std::to_string(sec) + "s";
    return std::to_string(sec) + "s";
}

}
