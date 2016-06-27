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
#include <mutex>
#include <string>
#include <thread>
#include <queue>

midi::MidiManager midiManager;
std::string selectedInputPort;
std::string selectedOutputPort;
std::map<std::string, bool> inputPortNamesMap;
std::map<std::string, bool> outputPortNamesMap;
std::deque<std::pair<midi::ChannelMessage, double>> inputLog;
std::deque<midi::ChannelMessage> outputLog;
std::mutex inputMutex;

void closePort() {
    for (auto& pair : inputPortNamesMap) {
        pair.second = false;
    }
    for (auto& pair : outputPortNamesMap) {
        pair.second = false;
    }
    midiManager.closePort();
}

void openPort() {
    closePort();
    inputPortNamesMap[selectedInputPort] = true;
    outputPortNamesMap[selectedOutputPort] = true;

    auto messageRecieved = [](const midi::ChannelMessage& message, const double& delay) {
        inputMutex.lock();
        inputLog.push_front({message, delay});
        inputMutex.unlock();
    };
    midiManager.openPort(selectedInputPort, selectedOutputPort, messageRecieved);
}

void refreshPorts() {
    midiManager.closePort();
    inputPortNamesMap.clear();
    outputPortNamesMap.clear();
    inputLog.clear();
    outputLog.clear();

    for (auto& portName : midiManager.getInputPortNames()) {
        inputPortNamesMap[portName] = false;
    }

    for (auto& portName : midiManager.getOutputPortNames()) {
        outputPortNamesMap[portName] = false;
    }

    if (inputPortNamesMap.size() > 0) {
        selectedInputPort = inputPortNamesMap.begin()->first;
    }

    if (outputPortNamesMap.size() > 0) {
        selectedOutputPort = outputPortNamesMap.begin()->first;
    }

    openPort();
}

void showInputs() {
    ImGui::BeginChild("inputs_header");
    ImGui::Text("Inputs");
    ImGui::Separator();

    ImGui::BeginChild("inputs");
    for (auto& pair : inputPortNamesMap) {
        auto portName = pair.first;
        auto selected = &pair.second;
        if (ImGui::Checkbox(portName.c_str(), selected)) {
            selectedInputPort = *selected ? portName : "";
            openPort();
        }
    }
    ImGui::EndChild();

    ImGui::EndChild();
}

void showOutputs() {
    ImGui::BeginChild("outputs_header");
    ImGui::Text("Outputs");
    ImGui::Separator();

    ImGui::BeginChild("outputs");
    for (auto& pair : outputPortNamesMap) {
        auto portName = pair.first;
        auto selected = &pair.second;
        if (ImGui::Checkbox(portName.c_str(), selected)) {
            selectedOutputPort = *selected ? portName : "";
            openPort();
        }
    }
    ImGui::EndChild();

    ImGui::EndChild();
}

void showOptions() {
    ImGui::BeginChild("options");
    ImGui::Text("Options");
    ImGui::Separator();

    if (ImGui::Button("Refresh Devices")) {
        refreshPorts();
    }

    ImGui::EndChild();
}

void showOutput() {
    ImGui::BeginChild("output");
    ImGui::Text("Output");
    ImGui::Separator();

    static const char* typeStrings[] = {
        "Note Off",
        "Note On",
        "Polyphonic Aftertouch",
        "Control Change",
        "Program Change",
        "Channel Aftertouch",
        "Pitch Wheel"
    };
    static const midi::ChannelMessage::Type typeValues[] = {
        midi::ChannelMessage::Type::NoteOff,
        midi::ChannelMessage::Type::NoteOn,
        midi::ChannelMessage::Type::PolyphonicAftertouch,
        midi::ChannelMessage::Type::ControlChange,
        midi::ChannelMessage::Type::ProgramChange,
        midi::ChannelMessage::Type::ChannelAftertouch,
        midi::ChannelMessage::Type::PitchWheel
    };
    static int typeIndex = 1;
    ImGui::Combo("Type", &typeIndex, typeStrings, sizeof(typeStrings)/sizeof(*typeStrings));

    static const char* channels[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
    static int channel = 0;
    ImGui::Combo("Channel", &channel, channels, sizeof(channels)/sizeof(*channels));

    static int dataByte1 = 0;
    ImGui::InputInt("Data 1", &dataByte1);
    if (dataByte1 < 0x00)
        dataByte1 = 0x00;
    if (dataByte1 > 0x7f)
        dataByte1 = 0x7f;

    static int dataByte2 = 0x7f;
    ImGui::InputInt("Data 2", &dataByte2);
    if (dataByte2 < 0x00)
        dataByte2 = 0x00;
    if (dataByte2 > 0x7f)
        dataByte2 = 0x7f;

    if (ImGui::Button("Send Midi")) {
        uint8_t typeByte = (uint8_t)typeValues[typeIndex];
        uint8_t channelByte = channel;
        uint8_t statusByte = typeByte | channelByte;
        midi::ChannelMessage message(statusByte, dataByte1, dataByte2);
        midiManager.sendMessage(message);
        outputLog.push_front(message);
    }

    ImGui::EndChild();
}

void showInputLog() {
    if(!inputMutex.try_lock())
        return;

    ImGui::BeginChild("input log");
    ImGui::Text("Input Log");

    ImGui::BeginChild("header", {0, 26});
    ImGui::Columns(5);
    ImGui::Text("Delay");   ImGui::NextColumn();
    ImGui::Text("Type");    ImGui::NextColumn();
    ImGui::Text("Channel"); ImGui::NextColumn();
    ImGui::Text("Data 1");  ImGui::NextColumn();
    ImGui::Text("Data 2");  ImGui::NextColumn();
    ImGui::Separator();
    ImGui::EndChild();

    ImGui::BeginChild("table");
    ImGui::Columns(5);
    for (auto& pair : inputLog) {
        const auto message = pair.first;
        const auto delay = pair.second;

        ImGui::Text(std::to_string(delay).c_str());             ImGui::NextColumn();
        ImGui::Text(message.typeString().c_str());              ImGui::NextColumn();
        ImGui::Text(std::to_string(message.channel()).c_str()); ImGui::NextColumn();
        ImGui::Text(std::to_string(message.byte1()).c_str());   ImGui::NextColumn();
        ImGui::Text(std::to_string(message.byte2()).c_str());   ImGui::NextColumn();
    }
    ImGui::EndChild();

    ImGui::EndChild();
    inputMutex.unlock();
}

void showOutputLog() {
    ImGui::BeginChild("output log", {0, 126});
    ImGui::Text("Output Log");

    ImGui::BeginChild("header", {0, 26});
    ImGui::Columns(4);
    ImGui::Text("Type");    ImGui::NextColumn();
    ImGui::Text("Channel"); ImGui::NextColumn();
    ImGui::Text("Data 1");  ImGui::NextColumn();
    ImGui::Text("Data 2");  ImGui::NextColumn();
    ImGui::Separator();
    ImGui::EndChild();

    ImGui::BeginChild("table");
    ImGui::Columns(4);
    for (auto& message : outputLog) {
        ImGui::Text(message.typeString().c_str());              ImGui::NextColumn();
        ImGui::Text(std::to_string(message.channel()).c_str()); ImGui::NextColumn();
        ImGui::Text(std::to_string(message.byte1()).c_str());   ImGui::NextColumn();
        ImGui::Text(std::to_string(message.byte2()).c_str());   ImGui::NextColumn();
    }
    ImGui::EndChild();

    ImGui::EndChild();
}

auto start = std::chrono::high_resolution_clock::now();

void sleep() {
    auto frameDuration = std::chrono::milliseconds(1000 / 60);
    auto now = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(now - start);
    if (elapsed < frameDuration) {
        auto sleepDuration = std::chrono::duration_cast<std::chrono::nanoseconds>(frameDuration - elapsed);
        std::this_thread::sleep_for(sleepDuration);
    }
    start = std::chrono::high_resolution_clock::now();
}

int main() {
    if (!glfwInit())
        exit(1);

    GLFWwindow* window = glfwCreateWindow(1000, 600, "Beagle", nullptr, nullptr);
	if (!window)
		exit(1);

    glfwMakeContextCurrent(window);
    ImGui_ImplGlfw_Init(window, true);

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.Fonts->AddFontFromMemoryCompressedTTF(droid_compressed_data, droid_compressed_size, 16);

    ImVec4 normal  = {0.90f, 0.90f, 0.90f, 1.0f};
    ImVec4 hovered = {0.80f, 0.80f, 0.80f, 1.0f};
    ImVec4 active  = {0.70f, 0.70f, 0.70f, 1.0f};

    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0;
    style.Colors[ImGuiCol_Text]                 = {0, 0, 0, 1};
    style.Colors[ImGuiCol_TextSelectedBg]       = hovered;
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
    style.Colors[ImGuiCol_ComboBg]              = normal;
    style.Colors[ImGuiCol_Header]               = active;
    style.Colors[ImGuiCol_HeaderHovered]        = hovered;
    style.Colors[ImGuiCol_HeaderActive]         = active;

    refreshPorts();

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

        ImGui::BeginChild("child", {0, 160});
        ImGui::Columns(4);
        showInputs(); ImGui::NextColumn();
        showOutputs(); ImGui::NextColumn();
        showOptions(); ImGui::NextColumn();
        showOutput(); ImGui::NextColumn();
        ImGui::EndChild();
        showOutputLog();
        ImGui::Dummy({0, 10});
        showInputLog();
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

    ImGui_ImplGlfw_Shutdown();
    glfwTerminate();
    
    return 0;
}
