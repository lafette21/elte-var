#include "gui.h"

namespace var {

gui::gui(const std::string& title, vec2 size):
    _glslVersion(GLSLVersion()),
    _dimensions(size)
{
    glfwSetErrorCallback([](int err, const char* msg) { logging::error("{} {}", err, msg); });

    if (not glfwInit()) {
        throw std::runtime_error("Failed to initialize GUI!");
    }

    _window = glfwCreateWindow(static_cast<int>(size.x()), static_cast<int>(size.y()), title.c_str(), nullptr, nullptr);
    if (_window == nullptr) {
        throw std::runtime_error("Failed to initialize window!");
    }

    glfwMakeContextCurrent(_window);

    logging::info("OpenGL Version: {}", std::string(reinterpret_cast<const char*>(glGetString(GL_VERSION))));

    // Enable vsync
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(_window, true);

    ImGui_ImplOpenGL2_Init();
}

gui::gui(gui&& other) noexcept {
    _window = other._window;
    _glslVersion = other._glslVersion;
    _dimensions = other._dimensions;

    other._window = nullptr;
}

gui::~gui() {
    if (_window == nullptr) {
        return;
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(_window);
    glfwTerminate();
}

std::string_view gui::GLSLVersion() {
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    return "#version 100";
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);      // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);                // Required on Mac
    return "#version 150";
#else
    // GL 3.0 + GLSL 130
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);   // 3.2+ only
    // glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);             // 3.0+ only
    return "#version 130";
#endif
}

} // namespace var
