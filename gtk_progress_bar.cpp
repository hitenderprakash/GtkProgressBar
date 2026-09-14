#include "gtk_progress_bar.h"

#include <gtk/gtk.h>
#include <glib.h>

#include <atomic>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <utility>

namespace SimpleGtkProgressBar {
namespace {

// A single queued instruction handed from the caller's thread to the GTK
// thread. `absolute == true` sets the bar to `percent`; otherwise `percent`
// is added to the current value.
struct UpdateCommand {
    bool absolute;
    int percent;
    bool hasLabel;
    std::string label;
};

GtkApplication* gApp = nullptr;
GtkWidget* gWindow = nullptr;
GtkWidget* gLabel = nullptr;
GtkWidget* gProgress = nullptr;

std::thread gLoopThread;
std::mutex gQueueMutex;
std::queue<UpdateCommand> gQueue;
std::atomic<bool> gStopRequested{false};
std::atomic<bool> gActive{false};

// Initial parameters consumed by the "activate" callback on the GTK thread.
std::string gInitTitle;
std::string gInitLabel;
int gInitPercent = 0;

int Clamp(int value) {
    if (value < 0) return 0;
    if (value > 100) return 100;
    return value;
}

// GTK thread only.
void ApplyProgress(int percent) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(gProgress), percent / 100.0);
    std::string text = std::to_string(percent) + "%";
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(gProgress), text.c_str());
}

// Drains all queued commands. Registered with g_timeout_add, so it runs on
// the GTK thread where touching widgets is safe.
gboolean DrainQueue(gpointer) {
    if (gStopRequested.load()) {
        return G_SOURCE_REMOVE;
    }

    std::queue<UpdateCommand> pending;
    {
        std::lock_guard<std::mutex> lock(gQueueMutex);
        std::swap(pending, gQueue);
    }

    while (!pending.empty()) {
        const UpdateCommand& cmd = pending.front();
        int current = static_cast<int>(
            gtk_progress_bar_get_fraction(GTK_PROGRESS_BAR(gProgress)) * 100.0 + 0.5);
        int target = cmd.absolute ? cmd.percent : current + cmd.percent;
        ApplyProgress(Clamp(target));
        if (cmd.hasLabel) {
            gtk_label_set_text(GTK_LABEL(gLabel), cmd.label.c_str());
        }
        pending.pop();
    }
    return G_SOURCE_CONTINUE;
}

// Window close button: request shutdown and let the default handler destroy
// the window.
gboolean OnDeleteEvent(GtkWidget*, GdkEvent*, gpointer) {
    gStopRequested.store(true);
    if (gApp) {
        g_application_quit(G_APPLICATION(gApp));
    }
    return FALSE;
}

void OnActivate(GtkApplication* app, gpointer) {
    gWindow = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(gWindow), gInitTitle.c_str());
    gtk_window_set_default_size(GTK_WINDOW(gWindow), 320, 90);
    g_signal_connect(gWindow, "delete-event", G_CALLBACK(OnDeleteEvent), nullptr);

    gLabel = gtk_label_new(gInitLabel.c_str());
    gProgress = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(gProgress), TRUE);
    ApplyProgress(Clamp(gInitPercent));

    GtkWidget* box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_box_pack_start(GTK_BOX(box), gProgress, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(box), gLabel, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(gWindow), box);

    g_timeout_add(100, DrainQueue, nullptr);
    gtk_widget_show_all(gWindow);
}

// Entry point of the background thread: owns the GTK application lifetime.
void RunLoop() {
    gApp = gtk_application_new("org.simple.gtkprogressbar", G_APPLICATION_NON_UNIQUE);
    g_signal_connect(gApp, "activate", G_CALLBACK(OnActivate), nullptr);
    g_application_run(G_APPLICATION(gApp), 0, nullptr);
    g_object_unref(gApp);
    gApp = nullptr;
    gActive.store(false);
}

// Scheduled onto the GTK thread from Destroy() so the loop quits itself
// instead of another thread poking GTK directly.
gboolean QuitOnGtkThread(gpointer) {
    if (gApp) {
        g_application_quit(G_APPLICATION(gApp));
    }
    return G_SOURCE_REMOVE;
}

} // namespace

bool Create(const std::string& windowTitle, const std::string& initialLabel, int initialPercent) {
    if (gActive.load()) {
        return false;
    }

    gInitTitle = windowTitle;
    gInitLabel = initialLabel;
    gInitPercent = initialPercent;
    gStopRequested.store(false);
    {
        std::lock_guard<std::mutex> lock(gQueueMutex);
        std::queue<UpdateCommand> empty;
        std::swap(gQueue, empty);
    }

    gActive.store(true);
    gLoopThread = std::thread(RunLoop);
    return true;
}

void Update(int incrementPercent, const std::string& labelText) {
    if (!gActive.load()) {
        return;
    }
    std::lock_guard<std::mutex> lock(gQueueMutex);
    gQueue.push(UpdateCommand{false, incrementPercent, !labelText.empty(), labelText});
}

void SetProgress(int percent, const std::string& labelText) {
    if (!gActive.load()) {
        return;
    }
    std::lock_guard<std::mutex> lock(gQueueMutex);
    gQueue.push(UpdateCommand{true, percent, !labelText.empty(), labelText});
}

void Destroy() {
    if (!gLoopThread.joinable()) {
        return;
    }
    gStopRequested.store(true);
    g_idle_add(QuitOnGtkThread, nullptr);
    gLoopThread.join();
    gActive.store(false);
}

bool IsActive() {
    return gActive.load();
}

} // namespace SimpleGtkProgressBar
