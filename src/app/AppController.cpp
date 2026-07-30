#include "AppController.hpp"
#include "Paths.hpp"
#include "storage/Storage.hpp"
#include "ui/Theme.hpp"
#include "ui/screens/ResultsScreen.hpp"
#include "ui/screens/HistoryScreen.hpp"
#include "ui/screens/StatsScreen.hpp"
#include "ui/screens/InfoScreen.hpp"
#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>
#include <functional>
#include <chrono>

namespace typr {

void AppController::startDownloadIfNeeded(const ContentPaths& paths) {
    if (m_downloadRunning.load()) return;

    auto sentencesUrl = ContentLocator::downloadUrlSentences();
    auto wordsUrl = ContentLocator::downloadUrlWords();
    auto sDest = paths.sentencesDb;
    auto wDest = paths.wordsDb;

    m_contentState->downloadState = DownloadState::Downloading;

    m_downloadRunning = true;
    if (m_downloadThread.joinable()) {
        m_downloadThread.join();
    }
    m_downloadThread = std::thread([this, sentencesUrl, wordsUrl, sDest, wDest] {
        bool sOk = ContentLocator::download(sentencesUrl, sDest);
        bool wOk = ContentLocator::download(wordsUrl, wDest);

        if (sOk && wOk) {
            m_contentPaths.found = true;
            m_contentPaths.sentencesDb = sDest;
            m_contentPaths.wordsDb = wDest;
            m_contentState->downloadState = DownloadState::Ready;
            applyProviderSelection();
        } else {
            m_contentState->downloadState = DownloadState::Failed;
        }

        m_downloadRunning = false;
    });
}

void AppController::applyProviderSelection() {
    auto& st = *m_contentState;
    if (!m_contentPaths.found) return;

    int builtins = (int)st.builtinTypes.size();
    if (st.selectedIndex < builtins) {
        auto type = st.builtinTypes[st.selectedIndex];
        m_engine.setContentProvider(
            ContentSource::makeProvider(type, m_contentPaths.sentencesDb, m_contentPaths.wordsDb));
    } else {
        int fileIdx = st.selectedIndex - builtins;
        if (fileIdx < (int)st.filePaths.size()) {
            m_engine.setContentProvider(
                ContentSource::makeProvider(ContentType::File,
                     m_contentPaths.sentencesDb, m_contentPaths.wordsDb,
                     st.filePaths[fileIdx]));
        }
    }
}

AppController::AppController(ftxui::ScreenInteractive& screen, Storage& storage)
    : m_screen(screen)
    , m_engine(cpptypr::EngineMode::Strict, 30)
    , m_storage(&storage)
{
    using namespace ftxui;
    using namespace ui;

    m_typingState = std::make_shared<TypingScreenState>();
    m_typingState->navigateToMain = [this] { navigateTo(ScreenType::Main); };
    m_typingState->navigateToResults = [this] { navigateTo(ScreenType::Results); };

    m_contentState = std::make_shared<ContentSelectorState>();
    m_contentPaths = ContentLocator::resolve();

    // Enable auto-save sessions
    m_engine.engine().setAutoSave(m_storage->repo(), true);

    // Wire navigation engine callbacks
    m_engineHandles.push_back(m_engine.engine().onFinished([this] {
        m_pendingNav.store(ScreenType::Results, std::memory_order_release);
    }));
    m_engineHandles.push_back(m_engine.engine().onTimeout([this] {
        m_pendingNav.store(ScreenType::Results, std::memory_order_release);
    }));
    m_engineHandles.push_back(m_engine.engine().onIncorrectKeystroke([this] {
        m_typingState->errorFlashActive.store(true, std::memory_order_release);
        m_typingState->errorFlashStartMs.store(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()
            ).count(),
            std::memory_order_release);
    }));

    // Main screen
    MainScreenCallbacks mainCB;
    mainCB.onStartPractice = [this] {
        m_engine.reset();
        applyProviderSelection();
        m_typingState->showIdleHint = true;
        navigateTo(ScreenType::Typing);
    };
    mainCB.onHistory = [this] { navigateTo(ScreenType::History); };
    mainCB.onStats = [this] { navigateTo(ScreenType::Stats); };
    mainCB.onInfo = [this] { navigateTo(ScreenType::Info); };
    mainCB.onQuit = [this] { m_screen.Exit(); };

    m_mainScreen = MainScreen(mainCB, m_contentState);

    // Typing screen
    m_typingScreen = TypingScreen(m_engine, m_typingState);

    // Results screen
    ResultsScreenCallbacks resultsCB;
    resultsCB.onRestart = [this] {
        m_engine.reset();
        navigateTo(ScreenType::Typing);
    };
    resultsCB.onNext = [this] {
        m_engine.reset();
        navigateTo(ScreenType::Typing);
    };
    resultsCB.onMain = [this] { navigateTo(ScreenType::Main); };
    m_resultsScreen = ResultsScreen(m_engine, *m_storage, resultsCB);

    {
        HistoryScreenCallbacks historyCB;
        historyCB.onMain = [this] { navigateTo(ScreenType::Main); };
        m_historyScreen = HistoryScreen(*m_storage, historyCB);
    }

    {
        StatsScreenCallbacks statsCB;
        statsCB.onMain = [this] { navigateTo(ScreenType::Main); };
        m_statsScreen = StatsScreen(*m_storage, statsCB);
    }

    {
        InfoScreenCallbacks infoCB;
        infoCB.onMain = [this] { navigateTo(ScreenType::Main); };
        m_infoScreen = InfoScreen(infoCB);
    }

    // Tab container for screen switching
    m_root = Container::Tab({
        m_mainScreen,
        m_typingScreen,
        m_resultsScreen,
        m_historyScreen,
        m_statsScreen,
        m_infoScreen,
    }, &m_activeTab);

    // Apply provider if DBs already found
    if (m_contentPaths.found) {
        m_contentState->downloadState = DownloadState::Ready;
        applyProviderSelection();
    } else {
        startDownloadIfNeeded(m_contentPaths);
    }

    // Start tick thread
    m_tickThread = std::thread([this] { tickLoop(); });
}

AppController::~AppController() {
    m_tickRunning = false;
    if (m_tickThread.joinable()) {
        m_tickThread.join();
    }
    m_downloadRunning = false;
    if (m_downloadThread.joinable()) {
        m_downloadThread.join();
    }
}

ftxui::Component AppController::root() {
    return m_root;
}

void AppController::navigateTo(ScreenType screen) {
    m_activeTab = static_cast<int>(screen);
}

void AppController::tickLoop() {
    while (m_tickRunning.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        auto snap = m_engine.getSnapshot();
        if (snap.state() == cpptypr::EngineState::Running) {
            m_engine.engine().tick();
        }

        auto pending = m_pendingNav.exchange(ScreenType::Main, std::memory_order_acq_rel);
        if (pending != ScreenType::Main) {
            m_activeTab = static_cast<int>(pending);
        }

        m_screen.PostEvent(ftxui::Event::Custom);
    }
}

}
