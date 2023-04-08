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
        glGenTextures(1, &_state.pitchTexture);
        glBindTexture(GL_TEXTURE_2D, _state.pitchTexture);
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
            _lastUpdate = std::chrono::steady_clock().now().time_since_epoch();
            // Menubar
            _gui.main_menu().menu("File")
                .item("Load", [this] {
                    _model.imagePath() = "/Users/lafette21/Downloads/video-assisted-referee/data/DSCF0137.jpeg";
                    _model.load();

                    glGenTextures(1, &_state.imageTexture);
                    glBindTexture(GL_TEXTURE_2D, _state.imageTexture);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                    // Set texture clamping method
                    const auto& image = _model.image();
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.cols, image.rows, 0, GL_RGBA, GL_UNSIGNED_BYTE, image.ptr());

                    _state.showImage = true;
                })
                .item("Save", [] { spdlog::info("Save"); });
            _gui.main_menu().menu("Edit")
                .item("Generate", [] { spdlog::info("Generate"); })
                .item("Reset", [] { spdlog::info("Reset"); });

            // State
            _state.width = _gui.width();
            _state.height = _gui.height();

            // Windows
            _gui.window("Debug", nullptr, ui::window_flag::None)
                .text(fmt::format("Logic time: {:.3f}ms", _delta.count() / 1'000'000.0))
                .text(fmt::format("Mouse pos: x={} y={}", ImGui::GetMousePos().x, ImGui::GetMousePos().y));

            _gui.window("Settings", nullptr, ui::window_flag::NoResize)
                .size(300, 150)
                .text("Pitch size")
                .input("width", &_model.pitchSize().x)
                .input("height", &_model.pitchSize().y)
                .text("Image")
                .input("path", &_model.imagePath());

            if (_state.showImage) {
                _gui.window("Image", nullptr, ui::window_flag::NoResize)
                    .callback([this] {
                        _state.windowPos = ImGui::GetWindowPos();
                        _state.availContent = ImGui::GetContentRegionAvail();
                        _state.curStartPos = ImGui::GetCursorStartPos();
                    })
                    .size(
                        static_cast<std::size_t>(_state.width / 2.1),
                        static_cast<std::size_t>((_state.width / 2.1) / _model.image().cols * _model.image().rows)
                    )
                    .image(
                        _state.imageTexture,
                        _state.pos = ImGui::GetCursorScreenPos(),
                        _state.contentSize = ImGui::GetContentRegionAvail() + _state.pos
                    )
                    .allow_overlap()
                    .invisible_button("Image sensor", ImGui::GetContentRegionAvail(), [this] {
                        const vec2 mousePos = ImGui::GetMousePos();
                        const vec2 mousePosInWindow = mousePos - _state.windowPos - _state.curStartPos;
                        const auto image = _model.image();
                        const double normX = static_cast<double>((image.cols / _state.contentSize.x) * mousePosInWindow.x);
                        const double normY = static_cast<double>((image.rows / _state.contentSize.y) * mousePosInWindow.y);

                        if (_state.imagePointsCurrentIdx != -1) {
                            _model.imagePoints()[_state.imagePointsCurrentIdx] = { normX, normY };
                        } else {
                            if (!_state.availableImagePointIdx.empty()) {
                                _model.imagePoints()[_state.availableImagePointIdx.top()] = { normX, normY };
                                _state.availableImagePointIdx.pop();
                            } else {
                                _model.imagePoints()[_state.nextAvailImagePointIdx++] = { normX, normY };
                            }
                        }

                        // draw
                    });
            }

            _gui.window("Pitch", nullptr, ui::window_flag::NoResize)
                .callback([this] {
                    _state.windowPos = ImGui::GetWindowPos();
                    _state.availContent = ImGui::GetContentRegionAvail();
                    _state.curStartPos = ImGui::GetCursorStartPos();
                })
                .size(
                    static_cast<std::size_t>(_state.width / 2.1),
                    static_cast<std::size_t>((_state.width / 2.1) / _model.pitch().cols * _model.pitch().rows)
                )
                .image(
                    _state.pitchTexture,
                    _state.pos = ImGui::GetCursorScreenPos(),
                    _state.pitchContentSize = ImGui::GetContentRegionAvail() + _state.pos
                )
                .allow_overlap()
                .invisible_button("Pitch sensor", ImGui::GetContentRegionAvail(), [this] {
                    const vec2 mousePos = ImGui::GetMousePos();
                    const vec2 mousePosInWindow = mousePos - _state.windowPos - _state.curStartPos;
                    const auto pitch = _model.pitch();
                    const double normX = static_cast<double>((pitch.cols / _state.contentSize.x) * mousePosInWindow.x);
                    const double normY = static_cast<double>((pitch.rows / _state.contentSize.y) * mousePosInWindow.y);

                    if (_state.pitchPointsCurrentIdx != -1) {
                        _model.pitchPoints()[_state.pitchPointsCurrentIdx] = { normX, normY };
                    } else {
                        if (!_state.availablePitchPointIdx.empty()) {
                            _model.pitchPoints()[_state.availablePitchPointIdx.top()] = { normX, normY };
                            _state.availablePitchPointIdx.pop();
                        } else {
                            _model.pitchPoints()[_state.nextAvailPitchPointIdx++] = { normX, normY };
                        }
                    }

                    // draw
                });
            _gui.window("Image points", nullptr, ui::window_flag::NoResize)
                .size(300, 200)
                .listbox("listbox", [this] {
                    _state.isImagePointSelected = false;

                    for (const auto& [key, value] : _model.imagePoints()) {
                        _state.isImagePointSelected = _state.imagePointsCurrentIdx == key;
                        std::string label = fmt::format("{} x={:.0f} y={:.0f}", key, value.x, value.y);

                        if (ImGui::Selectable(label.c_str(), &_state.isImagePointSelected)) {
                            _state.imagePointsCurrentIdx = key;
                        }

                        if (_state.isImagePointSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                })
                .button("Remove focus", [this] { _state.imagePointsCurrentIdx = -1; })
                .sameline()
                .button("Remove selected", [this] {
                    if (_state.imagePointsCurrentIdx != -1) {
                        _model.imagePoints().erase(_state.imagePointsCurrentIdx);
                        _state.availableImagePointIdx.push(_state.imagePointsCurrentIdx);
                        _state.imagePointsCurrentIdx = -1;
                        // TODO: draw
                    }
                });
            _gui.window("Pitch points", nullptr, ui::window_flag::NoResize)
                .size(300, 200)
                .listbox("listbox", [this] {
                    _state.isPitchPointSelected = false;

                    for (const auto& [key, value] : _model.pitchPoints()) {
                        _state.isPitchPointSelected = _state.pitchPointsCurrentIdx == key;
                        std::string label = fmt::format("{} x={:.0f} y={:.0f}", key, value.x, value.y);

                        if (ImGui::Selectable(label.c_str(), &_state.isPitchPointSelected)) {
                            _state.pitchPointsCurrentIdx = key;
                        }

                        if (_state.isPitchPointSelected) {
                            ImGui::SetItemDefaultFocus();
                        }
                    }
                })
                .button("Remove focus", [this] { _state.pitchPointsCurrentIdx = -1; })
                .sameline()
                .button("Remove selected", [this] {
                    if (_state.pitchPointsCurrentIdx != -1) {
                        _model.pitchPoints().erase(_state.pitchPointsCurrentIdx);
                        _state.availablePitchPointIdx.push(_state.pitchPointsCurrentIdx);
                        _state.pitchPointsCurrentIdx = -1;
                        // TODO: draw
                    }
                });

                _delta = std::chrono::steady_clock().now().time_since_epoch() - _lastUpdate;
        });
    }

private:
    struct state {
        MinPriorQueue availableImagePointIdx    = {};
        MinPriorQueue availablePitchPointIdx    = {};
        GLuint imageTexture, pitchTexture       = {};
        vec2 pos, availContent, windowPos, curStartPos, pitchContentSize, contentSize;
        int nextAvailImagePointIdx = 0;
        int nextAvailPitchPointIdx = 0;
        int imagePointsCurrentIdx   = -1;
        int pitchPointsCurrentIdx   = -1;
        int width, height;
        bool showImage              = false;
        bool isImagePointSelected   = false;
        bool isPitchPointSelected   = false;
    };

    std::chrono::nanoseconds _lastUpdate, _delta;
    gui _gui;
    model _model;
    state _state;
};

} // namespace var

#endif // APP_H
