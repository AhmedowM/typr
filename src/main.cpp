#include "app/AppController.hpp"
#include "app/Paths.hpp"
#include "storage/Storage.hpp"

#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>

int main() {
    typr::Paths::ensureDataDir();
    typr::Storage storage(typr::Paths::sessionsDb());

    auto screen = ftxui::ScreenInteractive::Fullscreen();

    typr::AppController app(screen, storage);

    auto component = app.root();
    component = ftxui::CatchEvent(component, [&](ftxui::Event event) {
        if (event == ftxui::Event::Custom) {
            return true;
        }
        if (event == ftxui::Event::CtrlQ) {
            screen.Exit();
            return true;
        }
        return false;
    });

    screen.Loop(component);
    return 0;
}
