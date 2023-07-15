set(EXEC_NAME elte_var)

add_library(imgui_backend
    ${CMAKE_SOURCE_DIR}/res/bindings/imgui_impl_glfw.cpp
    ${CMAKE_SOURCE_DIR}/res/bindings/imgui_impl_opengl2.cpp
)

add_executable(${EXEC_NAME}
    gui.cc
    main.cc
)

target_include_directories(${EXEC_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/res
)

code_analysis(${EXEC_NAME} PRIVATE)

target_link_libraries(${EXEC_NAME} PUBLIC
    ${CONAN_LIBS}
    imgui_backend
)
