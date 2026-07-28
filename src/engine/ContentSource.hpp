#pragma once

#include <cpptypr/content.hpp>
#include <filesystem>
#include <string>
#include <string_view>

namespace typr {

enum class ContentType {
    Sentences,
    CommonWords,
    RandomWords,
    File
};

struct ContentSource {
    /// Returns an empty provider as the initial placeholder for the engine.
    static cpptypr::ContentProvider makeProvider();

    /// DB-backed provider. sentencesDb/wordsDb are required for the built-in types.
    /// filePath is only used when type == File.
    static cpptypr::ContentProvider makeProvider(
        ContentType type,
        const std::filesystem::path& sentencesDb,
        const std::filesystem::path& wordsDb,
        const std::filesystem::path& filePath = {});

    static std::string_view label(ContentType type);
};

}
