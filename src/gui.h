#ifndef GUI_H
#define GUI_H

#include "bindings/imgui_impl_glfw.h"
#include "bindings/imgui_impl_opengl2.h"
#include "types.h"

#include <fmt/core.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <spdlog/spdlog.h>

#include <functional>
#include <string>
#include <vector>

namespace var {

namespace ui {

enum class window_flag {
    None                        = ImGuiWindowFlags_None,
    NoTitleBar                  = ImGuiWindowFlags_NoTitleBar,
    NoResize                    = ImGuiWindowFlags_NoResize,
    NoMove                      = ImGuiWindowFlags_NoMove,
    NoScrollbar                 = ImGuiWindowFlags_NoScrollbar,
    NoScrollWithMouse           = ImGuiWindowFlags_NoScrollWithMouse,
    NoCollapse                  = ImGuiWindowFlags_NoCollapse,
    AlwaysAutoResize            = ImGuiWindowFlags_AlwaysAutoResize,
    NoBackground                = ImGuiWindowFlags_NoBackground,
    NoSavedSettings             = ImGuiWindowFlags_NoSavedSettings,
    NoMouseInputs               = ImGuiWindowFlags_NoMouseInputs,
    MenuBar                     = ImGuiWindowFlags_MenuBar,
    HorizontalScrollbar         = ImGuiWindowFlags_HorizontalScrollbar,
    NoFocusOnAppearing          = ImGuiWindowFlags_NoFocusOnAppearing,
    NoBringToFrontOnFocus       = ImGuiWindowFlags_NoBringToFrontOnFocus,
    AlwaysVerticalScrollbar     = ImGuiWindowFlags_AlwaysVerticalScrollbar,
    AlwaysHorizontalScrollbar   = ImGuiWindowFlags_AlwaysHorizontalScrollbar,
    AlwaysUseWindowPadding      = ImGuiWindowFlags_AlwaysUseWindowPadding,
    NoNavInputs                 = ImGuiWindowFlags_NoNavInputs,
    NoNavFocus                  = ImGuiWindowFlags_NoNavFocus,
    UnsavedDocument             = ImGuiWindowFlags_UnsavedDocument,
    NoNav                       = ImGuiWindowFlags_NoNav,
    NoDecoration                = ImGuiWindowFlags_NoDecoration,
    NoInputs                    = ImGuiWindowFlags_NoInputs,
};

/**
 * @brief   Fundamental UI element holding other UI elements
 */
class window {
public:
    /*window(const std::string& title) {
        ImGui::Begin(title.c_str());
    }*/

    window(const std::string& title, bool* isOpen = nullptr, window_flag flag = window_flag::None) {
        ImGui::Begin(title.c_str(), isOpen, static_cast<int>(flag)); // TODO: More flag handling
    }

    ~window() {
        ImGui::End();
    }

    window(const window&)            = delete;
    window(window&&)                 = delete;
    window& operator=(const window&) = delete;
    window& operator=(window&&)      = delete;

    /**
     * @brief   Create a button which calls the `callback` upon clicking
     */
    window& button(const std::string& name, std::invocable auto&& callback) {
        if (ImGui::Button(name.c_str())) {
            std::invoke(callback);
        }
        return *this;
    }

    /**
     * @brief   Display text as is
     */
    window& text(const std::string& msg) {
        ImGui::Text("%s", msg.c_str());
        return *this;
    }

    /**
     * @brief   Display as formatted text
     */
    template <typename ...Args>
    window& text(const std::string& fmt, Args&&... args) {
        ImGui::Text("%s", fmt::vformat(fmt, fmt::make_format_args(std::forward<Args>(args))...).c_str());
        return *this;
    }

    /**
     * @brief   Create an input box
     */
    template <typename T>
    window& input(const std::string& label, T* value) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::InputInt(label.c_str(), value);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::InputFloat(label.c_str(), value);
        } else if constexpr (std::is_same_v<T, double>) {
            ImGui::InputDouble(label.c_str(), value);
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
                "Only the following types are supported for input: int, float, double, std::string"
            );
        }
        return *this;
    }

    /**
     * @brief   Create a checkbox
     */
    window& checkbox(const std::string& label, bool* value) {
        ImGui::Checkbox(label.c_str(), value);
        return *this;
    }

    window& size(std::size_t w, std::size_t h) {
        ImGui::SetWindowSize(ImVec2(w, h));
        return *this;
    }

    /**
     * @brief   TODO
     */
    window& separator() {
        ImGui::Separator();
        return *this;
    }

private:

};

/*
class popup {
public:
    popup(const std::string& name):
        _name(name),
        _active(ImGui::BeginPopup(name.c_str()))
    {}

    ~popup() {
        if (_active) {
            ImGui::EndPopup();
        }
    }

    void open() {
        ImGui::OpenPopup(_name.c_str());
    }

private:
    std::string _name;
    bool _active;
};
*/

class menu {
public:
    menu(const std::string& name):
        _active(ImGui::BeginMenu(name.c_str()))
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

    /**
     * @brief   Create an input box
     */
    template <typename T>
    menu& input(const std::string& label, T* value) {
        if constexpr (std::is_same_v<T, int>) {
            ImGui::InputInt(label.c_str(), value);
        } else if constexpr (std::is_same_v<T, float>) {
            ImGui::InputFloat(label.c_str(), value);
        } else if constexpr (std::is_same_v<T, double>) {
            ImGui::InputDouble(label.c_str(), value);
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
                "Only the following types are supported for input: int, float, double, std::string"
            );
        }
        return *this;
    }

    /**
     * @brief   Create a menu item
     */
    menu& item(const std::string& name, bool enabled, bool* selected = nullptr, const std::string& shortcut = "") {
        if (_active) {
            ImGui::MenuItem(name.c_str(), shortcut == "" ? nullptr : shortcut.c_str(), selected, enabled);
        }
        return *this;
    }

    /**
     * @brief   Create a menu item with a callback
     */
    menu& item(const std::string& name, std::invocable auto&& callback, bool* selected = nullptr, const std::string& shortcut = "") {
        if (_active) {
            if (ImGui::MenuItem(name.c_str(), shortcut == "" ? nullptr : shortcut.c_str(), selected)) {
                std::invoke(callback);
            }
        }
        return *this;
    }

private:
    bool _active;
};

class main_menu {
public:
    main_menu():
        _active(ImGui::BeginMainMenuBar())
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

    ui::menu menu(const std::string& name) {
        return ui::menu(name);
    }

private:
    bool _active;
};

} // namespace ui

/**
 * @brief   Wrapper class around GLFW and ImGUI
 */
class gui {
public:
    gui(const std::string& title, vec2 size);
    ~gui();

    gui(const gui&)             = delete;
    gui(gui&& other) noexcept;
    gui& operator=(const gui&)  = delete;
    gui& operator=(gui&&)       = delete;

    void run(std::invocable auto&& callback);

    ui::main_menu main_menu() {
        return ui::main_menu();
    }
/*
    ui::popup popup(const std::string& name) {
        return ui::popup(name);
    }
*/
    /**
     * @brief   Create a window with cache
     *
     * Converting data to the proper format should be abstracted away from the client code; it
     * must be done automatically with the provided interface (see more details at the member
     * functions of the `ui::window`).
     *ui_item
     * Data should be converted to the appropriate type only when it is needed
     * (hence the caching).
     */
    ui::window window(const std::string& title, bool* isOpen = nullptr, ui::window_flag flag = ui::window_flag::None) {
        /*if (not m_caches.contains(name)) {
            m_caches[name] = ui_state{};
        }*/
        return ui::window(title, isOpen, flag /*, m_caches[name]*/);
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
void gui::run(std::invocable auto&& callback) {
    while (not glfwWindowShouldClose(_window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        clear_color(colors::black);

        std::invoke(callback);

        ImGui::Render();
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(_window);
    }
}

inline void gui::clear_color(const vec4& color) {
    glClearColor(
        color.x * color.w,
        color.y * color.w,
        color.z * color.w,
        color.w
    );

    glClear(GL_COLOR_BUFFER_BIT);
}

} // namespace var

#endif // GUI_H
