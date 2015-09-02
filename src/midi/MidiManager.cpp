//  Copyright (c) 2015 hoseking. All rights reserved.

#include "MidiManager.h"

#include "MidiTypes.h"

namespace midi {

MidiManager::MidiManager() {
    mRtMidiIn.reset(new RtMidiIn());
    mRtMidiOut.reset(new RtMidiOut());
}

MidiManager::~MidiManager() {
    closePort();
}

std::vector<std::string> MidiManager::getPortNames() const {
    std::vector<std::string> portNames;

    auto portCount = mRtMidiIn->getPortCount();
    for (auto portNumber = 0; portNumber < portCount; ++portNumber) {
        auto portName = mRtMidiIn->getPortName(portNumber);
        portNames.push_back(portName);
    }

    return portNames;
}

bool MidiManager::openPort(std::string name, MidiRecievedFunction f) {
    closePort();

    const auto inputNumber = inputPortNumber(name);
    const auto outputNumber = outputPortNumber(name);
    if (inputNumber == -1 || outputNumber == -1) {
        return false;
    }

    try {
        mRtMidiOut->openPort(inputNumber);
        mRtMidiIn->openPort(outputNumber);
        mRtMidiIn->setCallback(&RtMidiCallback, this);
        mMidiRecievedFunction = f;
    } catch (RtMidiError e) {
        return false;
    }

    return true;
}

void MidiManager::closePort() {
    mRtMidiIn->cancelCallback();
    mRtMidiIn->closePort();
    mRtMidiOut->closePort();
    mMidiRecievedFunction = nullptr;
}

void MidiManager::sendMessage(const ChannelMessage& channelMessage) const {
    auto message = channelMessage.message();
    mRtMidiOut->sendMessage(&message);
}

void MidiManager::sendMessage(const SysExMessage& sysExMessage) const {
    auto message = sysExMessage.message();
    mRtMidiOut->sendMessage(&message);
}

int MidiManager::inputPortNumber(std::string name) const {
    auto portCount = mRtMidiIn->getPortCount();
    for (auto portNumber = 0; portNumber < portCount; ++portNumber) {
        auto portName = mRtMidiIn->getPortName(portNumber);
        if (portName == name)
            return portNumber;
    }

    return -1;
}

int MidiManager::outputPortNumber(std::string name) const {
    auto portCount = mRtMidiOut->getPortCount();
    for (auto portNumber = 0; portNumber < portCount; ++portNumber) {
        auto portName = mRtMidiOut->getPortName(portNumber);
        if (portName == name)
            return portNumber;
    }
    
    return -1;
}

void MidiManager::recievedMessage(const double& delay, std::vector<unsigned char>* message) const {
    const uint8_t statusByte = message->at(0);
    const uint8_t dataByte1 = message->at(1);
    const uint8_t dataByte2 = (message->size() > 2) ? message->at(2) : 0;
    mMidiRecievedFunction({statusByte, dataByte1, dataByte2}, delay);
}

}
