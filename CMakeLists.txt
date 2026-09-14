cmake_minimum_required(VERSION 3.10)

project(SimpleGtkProgressBar CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED gtk+-3.0)

# ---------------------------------------------------------------------------
# Static library: SimpleGtkProgressBar
# ---------------------------------------------------------------------------
add_library(SimpleGtkProgressBar STATIC
    gtk_progress_bar.cpp
)

target_include_directories(SimpleGtkProgressBar
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PUBLIC ${GTK_INCLUDE_DIRS}
)

target_compile_options(SimpleGtkProgressBar PRIVATE ${GTK_CFLAGS_OTHER})

# GTK lib directories (link_directories is used for CMake 3.10 compatibility).
link_directories(${GTK_LIBRARY_DIRS})

target_link_libraries(SimpleGtkProgressBar
    PUBLIC ${GTK_LIBRARIES}
    PUBLIC Threads::Threads
)

# ---------------------------------------------------------------------------
# Test application: LaunchProgressBarApp (links the static library)
# ---------------------------------------------------------------------------
add_executable(LaunchProgressBarApp
    LaunchProgressBarApp.cpp
)

target_link_libraries(LaunchProgressBarApp PRIVATE SimpleGtkProgressBar)
