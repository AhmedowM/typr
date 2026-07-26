#pragma once

#include <cpptypr/content.hpp>
#include <memory>

namespace typr {

struct ContentSource {
    static cpptypr::ContentProvider makeProvider();
};

}
