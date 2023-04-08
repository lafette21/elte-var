#include "app.h"

#include <fmt/format.h>

int main([[maybe_unused]] int argc, [[maybe_unused]] const char *argv[])
{
    try {
        var::app app;

        app.start();

    } catch (const std::exception& ex) {
        spdlog::error("{}", ex.what());
    }

    return 0;
}
