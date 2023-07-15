#ifndef APP_H
#define APP_H

#include "logging.h"
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
            _state.width = _gui.width();
            _state.height = _gui.height();

            renderMainMenu();
            renderWindows();

            _delta = std::chrono::steady_clock().now().time_since_epoch() - _lastUpdate;
        });
    }

    void renderMainMenu() {
        _gui.main_menu().menu("File")
            .item("Load", [this] { _state.loadPopupFlag = true; })
            .item("Save", [this] { _state.savePopupFlag = true; });

        _gui.main_menu().menu("Edit")
            .item("Generate", [this] {
                try {
                    auto [image, pitch] = _model.generate();
                    cv::cvtColor(image, image, cv::COLOR_RGBA2BGR);
                    cv::imshow("Pitch result", pitch);
                    cv::imshow("Image result", image);
                    cv::waitKey(0);
                    cv::destroyAllWindows();
                } catch (const std::exception& ex) {
                    logging::error("{}", ex.what());
                }
            })
            .item("Reset", [this] {
                _model.reset();
                renderImages();
            });

        _gui.main_menu().button("Debug", [this] { _state.debugWindowFlag = not _state.debugWindowFlag; });
    }

    void renderWindows() {
        if (_state.loadPopupFlag) {
            _gui.popup("Load")
                .input("path", &_model.imagePath())
                .separator()
                .button("Ok", vec2{ 120, 0 }, [this] {
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
                    _state.loadPopupFlag = false;
                    ImGui::CloseCurrentPopup();
                })
                .focus()
                .same_line()
                .button("Cancel", vec2{ 120, 0 }, [this] {
                    _state.loadPopupFlag = false;
                    ImGui::CloseCurrentPopup();
                })
                .open();
        }

        if (_state.savePopupFlag) {
            _gui.popup("Save")
                .input("path", &_state.savePath)
                .separator()
                .button("Ok", vec2{ 120, 0 }, [this] {
                    _model.save(_state.savePath);
                    _state.savePopupFlag = false;
                    ImGui::CloseCurrentPopup();
                })
                .focus()
                .same_line()
                .button("Cancel", vec2{ 120, 0 }, [this] {
                    _state.savePopupFlag = false;
                    ImGui::CloseCurrentPopup();
                })
                .open();
        }

        if (_state.debugWindowFlag) {
            _gui.window("Debug", ui::window_config{ ui::window_flag::None }, &_state.debugWindowFlag)
                .text("Logic time: {:.3f}ms", static_cast<double>(_delta.count()) / 1'000'000.0)
                .text("Mouse pos: x={} y={}", ImGui::GetMousePos().x, ImGui::GetMousePos().y)
                .text("debugWindowFlag: {}", _state.debugWindowFlag);
        }

        if (_state.showImage) {
            _gui.window("Image", ui::window_config{ ui::window_flag::NoResize })
                .callback([this] {
                    _state.windowPos = ImGui::GetWindowPos();
                    _state.curStartPos = ImGui::GetCursorStartPos();
                    _state.pos = ImGui::GetCursorScreenPos();
                    _state.imageContentSize = ImGui::GetContentRegionAvail();
                    if (_state.imageFirstFrame) {
                        _state.imageContentSize += vec2{ 1, 1 };
                        _state.imageFirstFrame = false;
                    }
                })
                .size({
                    static_cast<float>(_state.width / 2.1),
                    static_cast<float>((_state.width / 2.1) / _model.image().cols * _model.image().rows)
                })
                .image(
                    _state.imageTexture,
                    _state.pos,
                    _state.imageContentSize + _state.pos
                )
                .allow_overlap()
                .invisible_button("Image sensor", _state.imageContentSize, [this] {
                    const vec2 mousePos = ImGui::GetMousePos();
                    const vec2 mousePosInWindow = mousePos - _state.windowPos - _state.curStartPos;
                    const auto image = _model.image();
                    const vec2 imageSize = { static_cast<float>(image.cols), static_cast<float>(image.rows) };
                    const vec2 normPos = imageSize / _state.imageContentSize * mousePosInWindow;

                    if (_state.setAttacker) {
                        _model.attackerPos() = normPos;
                        _state.setAttacker = false;
                    } else if (_state.setDefender) {
                        _model.defenderPos() = normPos;
                        _state.setDefender = false;
                    } else {
                        if (_state.imagePointsCurrentIdx != -1) {
                            _model.imagePoints()[_state.imagePointsCurrentIdx] = normPos;
                        } else {
                            if (not _state.availableImagePointIdx.empty()) {
                                _model.imagePoints()[_state.availableImagePointIdx.top()] = normPos;
                                _state.availableImagePointIdx.pop();
                            } else {
                                _model.imagePoints()[_state.nextAvailImagePointIdx++] = normPos;
                            }
                        }
                    }

                    _model.draw();
                    renderImages();
                });
        }

        _gui.window("Pitch", ui::window_config{ ui::window_flag::NoResize })
            .callback([this] {
                _state.windowPos = ImGui::GetWindowPos();
                _state.curStartPos = ImGui::GetCursorStartPos();
                _state.pos = ImGui::GetCursorScreenPos();
                _state.pitchContentSize = ImGui::GetContentRegionAvail();
                if (_state.pitchFirstFrame) {
                    _state.pitchContentSize += vec2{ 1, 1 };
                    _state.pitchFirstFrame = false;
                }
            })
            .size({
                static_cast<float>(_state.width / 2.1),
                static_cast<float>((_state.width / 2.1) / _model.pitch().cols * _model.pitch().rows)
            })
            .image(
                _state.pitchTexture,
                _state.pos,
                _state.pitchContentSize + _state.pos
            )
            .allow_overlap()
            .invisible_button("Pitch sensor", _state.pitchContentSize, [this] {
                const vec2 mousePos = ImGui::GetMousePos();
                const vec2 mousePosInWindow = mousePos - _state.windowPos - _state.curStartPos;
                const auto pitch = _model.pitch();
                const vec2 pitchSize = { static_cast<float>(pitch.cols), static_cast<float>(pitch.rows) };
                const vec2 normPos = pitchSize / _state.pitchContentSize * mousePosInWindow;

                if (_state.pitchPointsCurrentIdx != -1) {
                    _model.pitchPoints()[_state.pitchPointsCurrentIdx] = normPos;
                } else {
                    if (not _state.availablePitchPointIdx.empty()) {
                        _model.pitchPoints()[_state.availablePitchPointIdx.top()] = normPos;
                        _state.availablePitchPointIdx.pop();
                    } else {
                        _model.pitchPoints()[_state.nextAvailPitchPointIdx++] = normPos;
                    }
                }

                _model.draw();
                renderImages();
            });

        _gui.window("Image points", ui::window_config{ ui::window_flag::NoResize })
            .size({ 300, 200 })
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
            .same_line()
            .button("Remove selected", [this] {
                if (_state.imagePointsCurrentIdx != -1) {
                    _model.imagePoints().erase(_state.imagePointsCurrentIdx);
                    _state.availableImagePointIdx.push(_state.imagePointsCurrentIdx);
                    _state.imagePointsCurrentIdx = -1;
                    _model.draw();
                    renderImages();
                }
            });

        _gui.window("Pitch points", ui::window_config{ ui::window_flag::NoResize })
            .size({ 300, 200 })
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
            .same_line()
            .button("Remove selected", [this] {
                if (_state.pitchPointsCurrentIdx != -1) {
                    _model.pitchPoints().erase(_state.pitchPointsCurrentIdx);
                    _state.availablePitchPointIdx.push(_state.pitchPointsCurrentIdx);
                    _state.pitchPointsCurrentIdx = -1;
                    _model.draw();
                    renderImages();
                }
            });

        _gui.window("Players", ui::window_config{ ui::window_flag::NoResize })
            .callback([this] {
                const auto image = _model.image();
                const vec2 imageSize = { static_cast<float>(image.cols), static_cast<float>(image.rows) };
                _state.attackerPos = _model.attackerPos() / (imageSize / _state.imageContentSize);
                _state.defenderPos = _model.defenderPos() / (imageSize / _state.imageContentSize);
            })
            .size({ 200, 160 })
            .text("Attacker")
            .text(
                "x={:.0f} y={:.0f}",
                _state.attackerPos.x(),
                _state.attackerPos.y()
            )
            .button("Set##Attacker", [this] { _state.setAttacker = true; })
            .same_line()
            .button("Reset##Attacker", [this] {
                _model.attackerPos() = { -1, -1 };
                _model.draw();
                renderImages();
            })
            .dummy({0, 10})
            .text("Defender")
            .text(
                "x={:.0f} y={:.0f}",
                _state.defenderPos.x(),
                _state.defenderPos.y()
            )
            .button("Set##Defender", [this] { _state.setDefender = true; })
            .same_line()
            .button("Reset##Defender", [this] {
                _model.defenderPos() = { -1, -1 };
                _model.draw();
                renderImages();
            });
    }

    void renderImages() {
        auto image = _model.image().clone();
        auto pitch = _model.pitch().clone();

        if (not image.empty()) {
            glBindTexture(GL_TEXTURE_2D, _state.imageTexture);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image.cols, image.rows, GL_RGBA, GL_UNSIGNED_BYTE, image.ptr());
        }

        glBindTexture(GL_TEXTURE_2D, _state.pitchTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, pitch.cols, pitch.rows, GL_RGBA, GL_UNSIGNED_BYTE, pitch.ptr());
    }

private:
    struct state {
        std::string savePath;
        MinPriorQueue availableImagePointIdx    = {};
        MinPriorQueue availablePitchPointIdx    = {};
        GLuint imageTexture, pitchTexture       = {};
        vec2 pos, windowPos, curStartPos, pitchContentSize, imageContentSize, attackerPos, defenderPos;
        int nextAvailImagePointIdx = 0;
        int nextAvailPitchPointIdx = 0;
        int imagePointsCurrentIdx   = -1;
        int pitchPointsCurrentIdx   = -1;
        int width, height;
        bool debugWindowFlag        = false;
        bool loadPopupFlag          = false;
        bool savePopupFlag          = false;
        bool imageFirstFrame        = true;
        bool pitchFirstFrame        = true;
        bool showImage              = false;
        bool isImagePointSelected   = false;
        bool isPitchPointSelected   = false;
        bool setAttacker            = false;
        bool setDefender            = false;
    };

    std::chrono::nanoseconds _lastUpdate, _delta;
    gui _gui;
    model _model;
    state _state;
};

} // namespace var

#endif // APP_H
