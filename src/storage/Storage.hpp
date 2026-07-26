#pragma once

#include <cpptypr.hpp>
#include <filesystem>
#include <memory>
#include <string_view>

namespace typr {

class Storage {
public:
    explicit Storage(std::filesystem::path dbPath);
    ~Storage() = default;

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;
    Storage(Storage&&) = delete;
    Storage& operator=(Storage&&) = delete;

    cpptypr::Repository& repo() { return *m_repo; }
    const cpptypr::Repository& repo() const { return *m_repo; }

private:
    std::unique_ptr<cpptypr::Repository> m_repo;
};

}
