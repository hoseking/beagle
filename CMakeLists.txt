cmake_minimum_required(VERSION 2.8.11)

project(beagle)

set(EXECUTABLE_OUTPUT_PATH bin)
set(CMAKE_CONFIGURATION_TYPES Debug Release)

set(CMAKE_CXX_FLAGS "-std=c++11 -Wall -Weffc++")
set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG=1")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")

# Add source
file(GLOB SRC "src/*.h" "src/*.cpp")
file(GLOB MIDI_SRC "src/midi/*.h" "src/midi/*.cpp")
file(GLOB RENDER_SRC "src/render/*.h" "src/render/*.cpp")
source_group("" FILES ${SRC})
source_group("midi" FILES ${MIDI_SRC})
source_group("render" FILES ${RENDER_SRC})
include_directories("src/midi")
include_directories("src/render")

# Add imgui
file(GLOB IMGUI_SRC "libs/imgui/*.h" "libs/imgui/*.cpp")
source_group("libs\\imgui" FILES ${IMGUI_SRC})
include_directories("libs/imgui")

# Add rtmidi
file(GLOB RTMIDI_SRC "libs/rtmidi/*.h" "libs/rtmidi/*.cpp")
source_group("libs\\rtmidi" FILES ${RTMIDI_SRC})
include_directories("libs/rtmidi")

# Add glfw
add_subdirectory("libs/glfw")
include_directories("libs/glfw/include")

add_executable(beagle ${SRC} ${RESOURCES} ${MIDI_SRC} ${RENDER_SRC} ${IMGUI_SRC} ${RTMIDI_SRC})
target_link_libraries(beagle glfw ${GLFW_LIBRARIES})

if(APPLE)
  set_property(TARGET beagle PROPERTY MACOSX_BUNDLE ON)
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")
  add_definitions("-D__MACOSX_CORE__")
  target_link_libraries(beagle "-framework CoreMIDI" "-framework CoreAudio")
else(WIN32)
  add_definitions("-D__WINDOWS_MM__")
  target_link_libraries(beagle "winmm.lib" "Rpcrt4.lib")
endif()
