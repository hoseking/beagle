cmake_minimum_required(VERSION 2.8.11)

project(beagle)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_CONFIGURATION_TYPES Debug Release)
set(CMAKE_CXX_FLAGS_RELEASE "-Os")

if(APPLE)
  set(CMAKE_CXX_FLAGS "-Wall -Weffc++")
  set(CMAKE_CXX_FLAGS_DEBUG "-g -DDEBUG=1")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8")
  add_definitions("-D__MACOSX_CORE__")
else(WIN32)
  add_definitions("-D__WINDOWS_MM__")
endif()

# Add source
file(GLOB SRC "src/*.h" "src/*.cpp")
file(GLOB MIDI_SRC "src/midi/*.h" "src/midi/*.cpp")
file(GLOB RENDER_SRC "src/render/*.h" "src/render/*.cpp")
source_group("" FILES ${SRC})
source_group("midi" FILES ${MIDI_SRC})
source_group("render" FILES ${RENDER_SRC})
include_directories("src/midi")
include_directories("src/render")

# Move submodules
file(COPY "submodules/" DESTINATION "libs")

# Add imgui
file(GLOB IMGUI_SRC "libs/imgui/*.h" "libs/imgui/*.cpp")
source_group("libs\\imgui" FILES ${IMGUI_SRC})
include_directories("libs/imgui")

# Add rtmidi
file(GLOB RTMIDI_SRC "libs/rtmidi/*.h" "libs/rtmidi/*.cpp")
source_group("libs\\rtmidi" FILES ${RTMIDI_SRC})
include_directories("libs/rtmidi")

# Add glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
add_subdirectory("libs/glfw")
include_directories("libs/glfw/include")

add_executable(beagle ${SRC} ${MIDI_SRC} ${RENDER_SRC} ${IMGUI_SRC} ${RTMIDI_SRC})
target_link_libraries(beagle glfw ${GLFW_LIBRARIES})

if(APPLE)
  set_property(TARGET beagle PROPERTY MACOSX_BUNDLE ON)
  target_link_libraries(beagle "-framework CoreMIDI" "-framework CoreAudio")
else(WIN32)
  set_property(TARGET beagle PROPERTY LINK_FLAGS "/ENTRY:mainCRTStartup")
  target_link_libraries(beagle "winmm.lib" "Rpcrt4.lib")
endif()
