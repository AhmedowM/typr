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

cpptypr::ContentProvider ContentSource::makeProvider(
    ContentType type,
    const std::filesystem::path& sentencesDb,
    const std::filesystem::path& wordsDb,
    const std::filesystem::path& filePath)
{
    using cpptypr::ContentMode;

    switch (type) {
    case ContentType::Sentences: {
        auto cp = cpptypr::ContentProvider::fromDatabase(sentencesDb.string());
        cp.setMode(ContentMode::Sentences);
        return cp;
    }
    case ContentType::CommonWords: {
        auto cp = cpptypr::ContentProvider::fromDatabase(wordsDb.string());
        cp.setMode(ContentMode::CommonWords);
        cp.setWordLengthRange(3, 8);
        return cp;
    }
    case ContentType::RandomWords: {
        auto cp = cpptypr::ContentProvider::fromDatabase(wordsDb.string());
        cp.setMode(ContentMode::RandomWords);
        cp.setWordLengthRange(3, 12);
        return cp;
    }
    case ContentType::File:
        return cpptypr::ContentProvider::fromFile(filePath.string());
    }

    return cpptypr::ContentProvider::fromString("");
}

std::string_view ContentSource::label(ContentType type) {
    switch (type) {
    case ContentType::Sentences:    return "Sentences";
    case ContentType::CommonWords:  return "Common Words";
    case ContentType::RandomWords:  return "Random Words";
    case ContentType::File:         return "File";
    }
    return "";
}

}
