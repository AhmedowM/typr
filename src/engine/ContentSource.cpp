#include "ContentSource.hpp"

namespace typr {

cpptypr::ContentProvider ContentSource::makeProvider() {
    return cpptypr::ContentProvider::fromString(
        "The quick brown fox jumps over the lazy dog. "
        "Pack my box with five dozen liquor jugs. "
        "How vexingly quick daft zebras jump. "
        "The five boxing wizards jump quickly. "
        "Sphinx of black quartz, judge my vow. "
        "Two driven jocks help fax my big quiz. "
        "Five quacking zephyrs jolt my wax bed. "
        "The public was amazed to view the quick ascent of the flying machine. "
        "A quivering ghost wrote letters of love and madness on the wall. "
        "The scholar carefully examined the ancient manuscript in the dim light."
    );
}

}
