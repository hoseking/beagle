//  Copyright (c) 2015 hoseking. All rights reserved.

#include "MidiManager.h"

#include "MidiTypes.h"

namespace midi {

MidiManager::MidiManager() {
    mRtMidiIn.reset(new RtMidiIn());
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

    const auto number = portNumber(name);
    try {
        mRtMidiIn->openPort(number);
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
    mMidiRecievedFunction = nullptr;
}

int MidiManager::portNumber(std::string name) const {
    auto portCount = mRtMidiIn->getPortCount();
    for (auto portNumber = 0; portNumber < portCount; ++portNumber) {
        auto portName = mRtMidiIn->getPortName(portNumber);
        if (portName == name)
            return portNumber;
    }

    return -1;
}

void MidiManager::recievedMessage(const double& delay, std::vector<unsigned char>* message) const {
    const uint8_t statusByte = message->at(0);
    const uint8_t dataByte1 = message->at(1);
    const uint8_t dataByte2 = (message->size() > 2) ? message->at(2) : 0;
    mMidiRecievedFunction({delay, statusByte, dataByte1, dataByte2});
}

}
