#include "app.h"
#include "logging.h"

int main([[maybe_unused]] int argc, [[maybe_unused]] const char *argv[])
{
    try {
        var::app app;

        app.start();
    } catch (const std::exception& ex) {
        logging::error("{}", ex.what());
    }

    return 0;
}
