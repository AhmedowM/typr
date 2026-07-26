#include "EngineBridge.hpp"
#include "ContentSource.hpp"

namespace typr {

EngineBridge::EngineBridge(cpptypr::EngineMode mode, uint16_t timeout)
    : m_logger(std::make_unique<cpptypr::Logger>(cpptypr::LogLevel::Warning, false))
    , m_provider(ContentSource::makeProvider())
    , m_engine(std::make_unique<cpptypr::Engine>(mode, m_provider, timeout))
{
    m_engine->setLogger(*m_logger);

    m_handles.push_back(m_engine->onStarted([this] {
        auto s = m_engine->stats();
        (void)s;
    }));

    m_handles.push_back(m_engine->onFinished([this] {
        auto s = m_engine->stats();
        (void)s;
    }));

    m_handles.push_back(m_engine->onTimeout([this] {
        auto s = m_engine->stats();
        (void)s;
    }));

    m_handles.push_back(m_engine->onIncorrectKeystroke([this] {
    }));
}

void EngineBridge::start() {
    m_engine->start();
}

void EngineBridge::stop() {
    m_engine->stop();
}

void EngineBridge::reset() {
    m_engine->reset();
}

void EngineBridge::keyPress(char key) {
    m_engine->keyPress(key);
}

void EngineBridge::backspacePress() {
    m_engine->backspacePress();
}

cpptypr::Snapshot EngineBridge::getSnapshot() {
    return m_engine->getSnapshot();
}

cpptypr::SessionStats EngineBridge::stats() {
    return m_engine->stats();
}

}
