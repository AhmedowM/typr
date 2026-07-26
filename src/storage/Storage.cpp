#include "Storage.hpp"

namespace typr {

Storage::Storage(std::filesystem::path dbPath)
    : m_repo(std::make_unique<cpptypr::Repository>(dbPath.string()))
{
}

}
