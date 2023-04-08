#ifndef APP_H
#define APP_H

#include "model.h"
#include "gui.h"

#include <imgui.h>
#include <opencv2/core.hpp>

#include <map>
#include <queue>

namespace var {

using MinPriorQueue = std::priority_queue<int, std::vector<int>, std::greater<int>>;

class app {
public:
    app():
        _gui("ELTEVar", { 1280, 720 }),
        _model("res/pitch.png")
    {
        glGenTextures(1, &_view.pitchTexture);
        glBindTexture(GL_TEXTURE_2D, _view.pitchTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Set texture clamping method
        const auto& pitch = _model.pitch();
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pitch.cols, pitch.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, pitch.ptr());
    }

    void start() {
        _gui.run([this] {
            // Menubar
            _gui.main_menu().menu("File")
                .item("Load", [/*this*/] { spdlog::info("Load"); /* _gui.popup("Cica").open(); */ })
                .item("Save", [] { spdlog::info("Save"); });
            _gui.main_menu().menu("Edit")
                .item("Generate", [] { spdlog::info("Generate"); })
                .item("Reset", [] { spdlog::info("Reset"); });

            // State
            _view.width = _gui.width();
            _view.height = _gui.height();

            // Windows
            if (_view.showImage) {
                _gui.window("Image", &_view.showImage, ui::window_flag::NoResize);
            }

            _gui.window("Debug", nullptr, ui::window_flag::None)
                .text(fmt::format("Mouse pos: x={} y={}", ImGui::GetMousePos().x, ImGui::GetMousePos().y));

            _gui.window("Pitch", nullptr, ui::window_flag::NoResize)
                .callback([this] {
                    _view.windowPos = ImGui::GetWindowPos();
                    _view.availContent = ImGui::GetContentRegionAvail();
                    _view.curStartPos = ImGui::GetCursorStartPos();
                })
                .size(
                    static_cast<std::size_t>(_view.width / 2.1),
                    static_cast<std::size_t>((_view.width / 2.1) / _model.pitch().cols * _model.pitch().rows)
                )
                .image(
                    _view.pitchTexture,
                    _view.pos = ImGui::GetCursorScreenPos(),
                    ImGui::GetContentRegionAvail() + _view.pos
                )
                .allow_overlap()
                .invisible_button("Pitch sensor", ImGui::GetContentRegionAvail(), [this] {
                    const vec2 mousePos = ImGui::GetMousePos();

                    const vec2 mousePosInWindow = mousePos - _view.windowPos - _view.curStartPos;

                    spdlog::info("pitch: x={}, y={}", _model.pitch().cols, _model.pitch().rows);
                    spdlog::info("mPIW: x={}, y={}", mousePosInWindow.x, mousePosInWindow.y);
                    spdlog::info("====================");
                });
            _gui.window("Image points", nullptr, ui::window_flag::NoResize)
                .size(300, 200);
            _gui.window("Pitch points", nullptr, ui::window_flag::NoResize)
                .size(300, 200)
                .listbox("listbox", [this] {
                    _view.isPitchPointSelected = false;

                    for (const auto& [key, value] : _model.pitchPoints()) {
                        _view.isPitchPointSelected = _view.pitchPointsCurrentIdx == key;
                        std::string label = fmt::format("{} x={} y={}", key, value.x, value.y);

                        if (ImGui::Selectable(label.c_str(), &_view.isPitchPointSelected)) {
                            _view.pitchPointsCurrentIdx = key;
                        }

                        if (_view.isPitchPointSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                })
                .button("Remove focus", [this] { _view.pitchPointsCurrentIdx = -1; })
                .sameline()
                .button("Remove selected", [this] {
                    if (_view.pitchPointsCurrentIdx != -1) {
                        _model.pitchPoints().erase(_view.pitchPointsCurrentIdx);
                        _view.availablePitchPointIdx.push(_view.pitchPointsCurrentIdx);
                        _view.pitchPointsCurrentIdx = -1;
                        // TODO: draw
                    }
                });
            _gui.window("Settings", nullptr, ui::window_flag::NoResize)
                .size(200, 100)
                .text("Pitch size")
                .input("width", &_model.pitchSize().x)
                .input("height", &_model.pitchSize().y);
        });
    }

private:
    struct menu {
        bool generate   = false;
        bool load       = false;
        bool reset      = false;
        bool save       = false;
    };

    struct view {
        MinPriorQueue availableImagePointIdx    = {};
        MinPriorQueue availablePitchPointIdx    = {};
        GLuint imageTexture, pitchTexture       = {};
        vec2 pos, availContent, windowPos, curStartPos;
        int imagePointsCurrentIdx   = -1;
        int pitchPointsCurrentIdx   = -1;
        int width, height;
        bool showImage              = true;
        bool isImagePointSelected   = false;
        bool isPitchPointSelected   = false;
    };

    gui _gui;
    // menu _menu;
    model _model;
    view _view;
};

} // namespace var

#endif // APP_H
