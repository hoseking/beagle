//  Copyright (c) 2015 hoseking. All rights reserved.

#include "font.h"
#include "imgui_impl_glfw.h"
#include "MidiManager.h"
#include "MidiTypes.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <chrono>
#include <iostream>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <queue>

midi::MidiManager midiManager;
std::map<std::string, bool> portNamesMap;
std::deque<midi::ChannelMessage> messageLog;

void closePort() {
    for (auto& pair : portNamesMap) {
        pair.second = false;
    }
    midiManager.closePort();
}

void openPort(const std::string& portName) {
    closePort();
    portNamesMap[portName] = true;

    auto messageRecieved = [](const midi::ChannelMessage& message) {
        messageLog.push_front(message);
        if (messageLog.size() > 200) {
            messageLog.pop_back();
        }
    };
    midiManager.openPort(portName, messageRecieved);
}

void refreshPorts() {
    midiManager.closePort();
    portNamesMap.clear();
    messageLog.clear();
    for (auto& portName : midiManager.getPortNames()) {
        portNamesMap[portName] = false;
    }
    if (portNamesMap.begin() != portNamesMap.end()) {
        auto it = portNamesMap.begin();
        openPort(it->first);
        it->second = true;
    }
}

void showDevices() {
    ImGui::BeginChild("devices_header");
    ImGui::Text("Devices");
    ImGui::Separator();
    ImGui::BeginChild("devices");

    for (auto& pair : portNamesMap) {
        auto portName = pair.first;
        auto selected = &pair.second;
        if (ImGui::Checkbox(portName.c_str(), selected)) {
            if (*selected) {
                openPort(portName);
            } else {
                closePort();
            }
        }
    }

    ImGui::EndChild();
    ImGui::EndChild();
}

void showOptions() {
    ImGui::BeginChild("options");
    ImGui::Text("Options");
    ImGui::Separator();

    if (ImGui::Button("Refresh Ports")) {
        refreshPorts();
    }

    ImGui::EndChild();
}

void showLog() {
    ImGui::Text("Input Log");

    ImGui::BeginChild("header", {0, 26});
    ImGui::Columns(5);
    ImGui::Text("Delay"); ImGui::NextColumn();
    ImGui::Text("Type"); ImGui::NextColumn();
    ImGui::Text("Channel"); ImGui::NextColumn();
    ImGui::Text("Data 1"); ImGui::NextColumn();
    ImGui::Text("Data 2"); ImGui::NextColumn();
    ImGui::Separator();
    ImGui::EndChild();

    ImGui::BeginChild("table");
    ImGui::Columns(5);
    for (auto& message : messageLog) {
        ImGui::Text(std::to_string(message.delay()).c_str());   ImGui::NextColumn();
        ImGui::Text(message.typeString().c_str());              ImGui::NextColumn();
        ImGui::Text(std::to_string(message.channel()).c_str()); ImGui::NextColumn();
        ImGui::Text(std::to_string(message.byte1()).c_str());   ImGui::NextColumn();
        ImGui::Text(std::to_string(message.byte2()).c_str());   ImGui::NextColumn();
    }
    ImGui::EndChild();
}

auto start = std::chrono::high_resolution_clock::now();

void sleep() {
    auto frame_duration = std::chrono::milliseconds(1000 / 60);
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
    if (elapsed < frame_duration) {
        auto sleep_duration = std::chrono::duration_cast<std::chrono::nanoseconds>(frame_duration - elapsed);
        std::this_thread::sleep_for(sleep_duration);
    }
    start = std::chrono::high_resolution_clock::now();
}

int main() {
    // Setup window
    if (!glfwInit())
        exit(1);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Beagle", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Setup ImGui binding
    ImGui_ImplGlfw_Init(window, true);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromMemoryCompressedTTF(droid_compressed_data, droid_compressed_size, 16);

    ImVec4 normal  = {0.90, 0.90, 0.90, 0.5};
    ImVec4 hovered = {0.90, 0.90, 0.90, 1.0};
    ImVec4 active  = {0.80, 0.80, 0.80, 1.0};

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0;
    style.Colors[ImGuiCol_Text]                 = {0, 0, 0, 1};
    style.Colors[ImGuiCol_WindowBg]             = {1, 1, 1, 1};
    style.Colors[ImGuiCol_Column]               = {0, 0, 0, 0};
    style.Colors[ImGuiCol_ColumnHovered]        = {0, 0, 0, 0};
    style.Colors[ImGuiCol_CheckMark]            = {0, 0, 0, 1};
    style.Colors[ImGuiCol_Button]               = normal;
    style.Colors[ImGuiCol_ButtonHovered]        = hovered;
    style.Colors[ImGuiCol_ButtonActive]         = active;
    style.Colors[ImGuiCol_FrameBg]              = normal;
    style.Colors[ImGuiCol_FrameBgHovered]       = hovered;
    style.Colors[ImGuiCol_FrameBgActive]        = active;
    style.Colors[ImGuiCol_ScrollbarBg]          = normal;
    style.Colors[ImGuiCol_ScrollbarGrab]        = active;
    style.Colors[ImGuiCol_ScrollbarGrabHovered] = active;
    style.Colors[ImGuiCol_ScrollbarGrabActive]  = {0, 0, 0, 1};

    refreshPorts();

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        ImGui_ImplGlfw_NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y));

        ImGuiWindowFlags flags = 0;
        flags |= ImGuiWindowFlags_NoTitleBar;
        flags |= ImGuiWindowFlags_NoMove;
        flags |= ImGuiWindowFlags_NoCollapse;
        flags |= ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin("main", nullptr, {0, 0}, 1, flags);

        ImGui::BeginChild("sub1", {0, 140});
        ImGui::Columns(2);
        showDevices(); ImGui::NextColumn();
        showOptions(); ImGui::NextColumn();
        ImGui::EndChild();
        ImGui::Dummy({0, 20});
        showLog();
        ImGui::End();

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui::Render();
        glfwSwapBuffers(window);
        sleep();
    }

    // Cleanup
    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();
    
    return 0;
}
