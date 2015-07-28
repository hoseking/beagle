//  Copyright (c) 2015 hoseking. All rights reserved.

#pragma once

#include "MidiTypes.h"

#include <RtMidi.h>

#include <memory>
#include <vector>
#include <string>

namespace midi {

class MidiManager {
public:
    MidiManager();
    ~MidiManager();

    std::vector<std::string> getPortNames() const;
    bool openPort(std::string name, MidiRecievedFunction f);
    void closePort();
    
private:
    int portNumber(std::string name) const;
    void recievedMessage(const double& delay, std::vector<unsigned char>* message) const;

private:
    std::unique_ptr<RtMidiIn> mRtMidiIn = nullptr;
    MidiRecievedFunction mMidiRecievedFunction;

private:
    static void RtMidiCallback(double delay, std::vector<unsigned char>* message, void* userData) {
        MidiManager* midiManager = static_cast<MidiManager*>(userData);
        midiManager->recievedMessage(delay, message);
    }
};

}