#ifndef APP_H
#define APP_H

#include "gui.h"

namespace var {

class app {
public:
    app(): _gui("ELTEVar", { 1280, 720 }) {}

    void start() {
        _gui.run([this] {
            // Menubar
            _gui.main_menu().menu("File")
                .item("Load", [this] { spdlog::info("Load"); /* _gui.popup("Cica").open(); */ })
                .item("Save", [] { spdlog::info("Save"); });
            _gui.main_menu().menu("Edit")
                .item("Generate", [] { spdlog::info("Generate"); })
                .item("Reset", [] { spdlog::info("Reset"); });
            _gui.main_menu().menu("Settings")
                .item("Pitch size", false)
                .input("width", &_model.pitchWidth)
                .input("height", &_model.pitchHeight);

            if (_model.showImage) {
                _gui.window("Image", &_model.showImage, var::ui::window_flag::NoResize);
            }

            if (_model.showPitch) {
                _gui.window("Pitch", &_model.showPitch, var::ui::window_flag::NoResize);
            }

            _gui.window("Image points", nullptr, var::ui::window_flag::NoResize)
                .size(300, 200);
            _gui.window("Pitch points", nullptr, var::ui::window_flag::NoResize)
                .size(300, 200);
        });
    }

private:
    struct menu {
        bool generate   = false;
        bool load       = false;
        bool reset      = false;
        bool save       = false;
    };

    struct model {
        double pitchWidth   = 0;
        double pitchHeight  = 0;
        bool showImage      = true;
        bool showPitch      = true;
    };

    gui _gui;
    // menu _menu;
    model _model;
};

} // namespace var

#endif // APP_H
