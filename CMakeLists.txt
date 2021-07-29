
cmake_minimum_required(VERSION 3.10)
set (CMAKE_CXX_STANDARD 11)

project(SimpleGtkProgressBar)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK REQUIRED "gtk+-3.0")


include_directories(${CMAKE_CURRENT_LIST_DIR} ${GTK_INCLUDE_DIRS})

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

set(${PROJECT_NAME}_SOURCE_FILES 
    "${CMAKE_CURRENT_LIST_DIR}/gtk_progress_bar.cpp"
)
add_executable(${PROJECT_NAME}  ${${PROJECT_NAME}_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${GTK_LIBRARIES})

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

project(LaunchProgressBarApp)
add_executable(${PROJECT_NAME}  LaunchProgressBarApp.cpp)