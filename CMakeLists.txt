cmake_minimum_required(VERSION 3.20)
project(i3space VERSION 0.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
find_package(X11 REQUIRED)
pkg_check_modules(SIMDJSON REQUIRED simdjson)
pkg_check_modules(XFT REQUIRED xft)
pkg_check_modules(XRENDER REQUIRED xrender)
pkg_check_modules(FONTCONFIG REQUIRED fontconfig)

add_executable(i3space
  src/main.cpp
  src/i3_ipc.cpp
  src/tree_flatten.cpp
  src/config.cpp
  src/first_run.cpp
  src/locale.cpp
  src/print_active.cpp
  src/toggle.cpp
  src/x11_strip.cpp
  src/layout_horizontal.cpp
  src/input_keyboard.cpp
  src/single_ws_toast.cpp
  src/compositor_detect.cpp
  src/fade.cpp
  src/mouse_input.cpp
)

target_include_directories(i3space PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${XFT_INCLUDE_DIRS}
  ${XRENDER_INCLUDE_DIRS}
  ${FONTCONFIG_INCLUDE_DIRS}
  ${SIMDJSON_INCLUDE_DIRS}
)

target_link_libraries(i3space PRIVATE
  ${SIMDJSON_LIBRARIES}
  ${X11_LIBRARIES}
  ${XFT_LIBRARIES}
  ${XRENDER_LIBRARIES}
  ${FONTCONFIG_LIBRARIES}
  Xext
)

target_compile_options(i3space PRIVATE -Wall -Wextra -O2)

install(TARGETS i3space RUNTIME DESTINATION bin)
