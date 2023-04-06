#include "app.h"

#include <fmt/format.h>

#include <iostream>

int main([[maybe_unused]] int argc, [[maybe_unused]] const char *argv[])
{
    std::puts(fmt::format("Example {}", "project").c_str());

    try {
        var::app app;

        app.start();

    } catch (const std::exception& ex) {
        spdlog::error("{}", ex.what());
    }

    return 0;
}
