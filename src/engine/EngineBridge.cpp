#include "EngineBridge.hpp"
#include "ContentSource.hpp"

namespace typr {

EngineBridge::EngineBridge(cpptypr::EngineMode mode, uint16_t timeout)
    : m_logger(std::make_unique<cpptypr::Logger>(cpptypr::LogLevel::Warning, false))
    , m_provider(ContentSource::makeProvider())
    , m_engine(std::make_unique<cpptypr::Engine>(mode, m_provider, timeout))
{
    m_engine->setLogger(*m_logger);

}

void EngineBridge::start() {
    try {
        m_engine->start();
    } catch (const std::exception&) {
    }
}

void EngineBridge::stop() {
    try {
        m_engine->stop();
    } catch (const std::exception&) {
    }
}

void EngineBridge::reset() {
    try {
        m_engine->reset();
    } catch (const std::exception&) {
    }
}

void EngineBridge::keyPress(char key) {
    try {
        m_engine->keyPress(key);
    } catch (const std::exception&) {
    }
}

void EngineBridge::backspacePress() {
    try {
        m_engine->backspacePress();
    } catch (const std::exception&) {
    }
}

cpptypr::Snapshot EngineBridge::getSnapshot() {
    return m_engine->getSnapshot();
}

cpptypr::SessionStats EngineBridge::stats() {
    try {
        return m_engine->stats();
    } catch (const std::exception&) {
        return {};
    }
}

void EngineBridge::setContentProvider(cpptypr::ContentProvider provider) {
    try {
        m_provider = std::move(provider);
        m_engine->setContentProvider(m_provider);
    } catch (const std::exception&) {
    }
}

}

