#pragma once

#include <cpptypr.hpp>
#include <memory>
#include <vector>

namespace typr {

class EngineBridge {
public:
    EngineBridge(cpptypr::EngineMode mode, uint16_t timeout);
    ~EngineBridge() = default;

    EngineBridge(const EngineBridge&) = delete;
    EngineBridge& operator=(const EngineBridge&) = delete;
    EngineBridge(EngineBridge&&) = default;
    EngineBridge& operator=(EngineBridge&&) = default;

    void start();
    void stop();
    void reset();

    void keyPress(char key);
    void backspacePress();

    cpptypr::Snapshot getSnapshot();
    cpptypr::SessionStats stats();

    void setContentProvider(cpptypr::ContentProvider provider);

    cpptypr::Engine& engine() { return *m_engine; }
    const cpptypr::Engine& engine() const { return *m_engine; }

private:
    std::unique_ptr<cpptypr::Logger> m_logger;
    cpptypr::ContentProvider m_provider;
    std::unique_ptr<cpptypr::Engine> m_engine;
    std::vector<cpptypr::CallbackHandle> m_handles;
};

}
