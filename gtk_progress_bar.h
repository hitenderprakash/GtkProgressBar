#pragma once

#include <string>

// SimpleGtkProgressBar
// ---------------------
// A tiny, self-contained progress-bar widget backed by GTK3.
//
// The GTK event loop runs on its own background thread, so all functions
// below are non-blocking and safe to call from an application's main thread.
// The public interface intentionally exposes no GTK types, so consumers do
// not need GTK headers to use the library (only to link against it).
//
// Typical usage:
//     SimpleGtkProgressBar::Create("Installing", "Starting...", 0);
//     SimpleGtkProgressBar::Update(25, "Copying files...");
//     SimpleGtkProgressBar::SetProgress(100, "Done");
//     SimpleGtkProgressBar::Destroy();
namespace SimpleGtkProgressBar {

// Creates and shows the progress-bar window and starts the GTK event loop on
// a dedicated background thread. Returns false if a progress bar is already
// active. `initialPercent` is clamped to the range [0, 100].
bool Create(const std::string& windowTitle,
            const std::string& initialLabel = "",
            int initialPercent = 0);

// Advances the bar by `incrementPercent` (the result is clamped to [0, 100]).
// When `labelText` is non-empty the message label is updated too. Thread-safe;
// a no-op when no progress bar is active.
void Update(int incrementPercent, const std::string& labelText = "");

// Sets the bar to an absolute `percent` (clamped to [0, 100]). When
// `labelText` is non-empty the message label is updated too. Thread-safe;
// a no-op when no progress bar is active.
void SetProgress(int percent, const std::string& labelText = "");

// Closes the window, stops the GTK loop and joins the background thread.
// Safe to call multiple times.
void Destroy();

// Returns true while the progress-bar window is active.
bool IsActive();

} // namespace SimpleGtkProgressBar
