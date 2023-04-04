#ifndef WINDOW_H
#define WINDOW_H

#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl3.h"
#include "types.h"

#include <fmt/core.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <string>
#include <functional>

namespace var {

namespace ui {

/**
 * @brief   Fundamental UI element holding other UI elements
 */
class widget {
public:
    widget(const std::string& title) {
        ImGui::Begin(title.c_str());
    }

    ~widget() {
        ImGui::End();
    }

    widget(const widget&)            = delete;
    widget(widget&&)                 = delete;
    widget& operator=(const widget&) = delete;
    widget& operator=(widget&&)      = delete;

    /**
     * @brief   Create a button which calls the `callback` upon clicking
     */
    widget& button(const std::string& name, std::invocable auto&& callback) {
        if (ImGui::Button(name.c_str())) {
            std::invoke(callback);
        }
        return *this;
    }

    /**
     * @brief   Display text as is
     */
    widget& text(const std::string& msg) {
        ImGui::Text("%s", msg.c_str());
        return *this;
    }

    /**
     * @brief   Display as formatted text
     */
    template <typename ...Args>
    widget& text(const std::string& fmt, Args&&... args) {
        ImGui::Text(fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args))...).c_str());
        return *this;
    }

    /**
     * @brief   Create an input box
     */
    template <typename T>
    widget& input(const std::string& label, T* value) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::InputInt(label.c_str(), value);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::InputFloat(label.c_str(), value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            ImGui::InputText(
                label.c_str(),
                value->data(),
                value->capacity() + 1,
                ImGuiInputTextFlags_CallbackResize,
                [](ImGuiInputTextCallbackData* data) -> int {
                    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
                        std::string* str = static_cast<std::string*>(data->UserData);
                        str->resize(static_cast<std::size_t>(data->BufTextLen));
                        data->Buf = str->data();
                    }
                    return 0;
                },
            value
            );
        } else {
            static_assert(
                std::is_same_v<T, void> && !std::is_same_v<T, void>,
                "Only the following types are supported for input: int, float, std::string"
            );
        }
        return *this;
    }

    /**
     * @brief   Create a checkbox
     */
    widget& checkbox(const std::string& label, bool* value) {
        ImGui::Checkbox(label.c_str(), value);
        return *this;
    }

private:

};

class menu {
public:
    menu(const std::string& name)
        : _active(ImGui::BeginMenu(name.c_str()))
    {}

    ~menu() {
        if (_active) {
            ImGui::EndMenu();
        }
    }

    menu(const menu&) = delete;
    menu(menu&&) = delete;
    menu& operator=(const menu&) = delete;
    menu& operator=(menu&&) = delete;

    menu& item(const std::string& name, bool* selected, const std::string& shortcut = "") {
        if (_active) {
            ImGui::MenuItem(name.c_str(), shortcut == "" ? nullptr : shortcut.c_str(), selected);
        }
        return *this;
    }

private:
    bool _active;
};

class main_menu {
public:
    main_menu()
        : _active(ImGui::BeginMainMenuBar())
    {}

    ~main_menu() {
        if (_active) {
            ImGui::EndMainMenuBar();
        }
    }

    main_menu(const main_menu&) = delete;
    main_menu(main_menu&&) = delete;
    main_menu& operator=(const main_menu&) = delete;
    main_menu& operator=(main_menu&&) = delete;

    menu menu(const std::string& name) {
        return ui::menu(name);
    }

private:
    bool _active;
};

} // namespace ui

/**
 * @brief   Wrapper class around GLFW and ImGUI
 */
class window {
public:
    window(const std::string& title, vec2 pos);
    ~window();

    window(const window&)             = delete;
    window(window&& other) noexcept;
    window& operator=(const window&)  = delete;
    window& operator=(window&&)       = delete;

    void run(std::invocable auto&& callback);

    ui::main_menu main_menu() {
        return ui::main_menu();
    }

    /**
     * @brief   Create a widget with cache
     *
     * Converting data to the proper format should be abstracted away from the client code; it
     * must be done automatically with the provided interface (see more details at the member
     * functions of the `widget`).
     *ui_item
     * Data should be converted to the appropriate type only when it is needed
     * (hence the caching).
     */
    ui::widget widget(const std::string& name) {
        /*if (not m_caches.contains(name)) {
            m_caches[name] = ui_state{};
        }*/
        return ui::widget(name/*, m_caches[name]*/);
    }

    [[nodiscard]] auto dimensions() const noexcept {
        return _dimensions;
    }

private:
    GLFWwindow* _window = nullptr;
    std::string_view _glslVersion;
    vec2 _dimensions;

    // std::map<std::string, ui_state> _caches;       // TODO(perf,data-structure): flatmap

    static void clear_color(const vec4& color);
    static std::string_view GLSLVersion();
};

/**
 * @brief   Event-loop
 */
void window::run(std::invocable auto&& callback) {
    while (not glfwWindowShouldClose(_window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        clear_color(colors::black);

        std::invoke(callback);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(_window);
    }
}

inline void window::clear_color(const vec4& color) {
    glClearColor(
        color.x * color.w,
        color.y * color.w,
        color.z * color.w,
        color.w
    );

    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace var

#endif // WINDOW_H
