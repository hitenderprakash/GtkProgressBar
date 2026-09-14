// Third-party test application for the SimpleGtkProgressBar static library.
//
// It links against the library and drives it purely through the public
// interface in gtk_progress_bar.h -- no GTK headers required here.

#include "gtk_progress_bar.h"

#include <chrono>
#include <thread>

namespace {
void Wait(int seconds) {
    std::this_thread::sleep_for(std::chrono::seconds(seconds));
}
} // namespace

int main() {
    namespace pb = SimpleGtkProgressBar;

    if (!pb::Create("Installing Application", "Starting...", 0)) {
        return 1;
    }

    Wait(1);
    pb::Update(10, "Initializing...");
    Wait(2);
    pb::Update(20, "Copying files...");
    Wait(2);
    pb::Update(20, "Configuring components...");
    Wait(2);
    pb::Update(20, "Registering services...");
    Wait(2);
    pb::Update(20, "Almost there...");
    Wait(2);
    pb::SetProgress(100, "Finishing...");
    Wait(2);

    pb::Destroy();
    return 0;
}
