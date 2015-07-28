//  Copyright (c) 2015 hoseking. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
#include <string>

namespace midi {

class ChannelMessage {
public:
    enum class Type : uint8_t {
        NoteOff = 0x80,
        NoteOn = 0x90,
        PolyphonicAftertouch = 0xA0,
        ControlChange = 0xB0,
        ProgramChange = 0xC0,
        ChannelAftertouch = 0xD0,
        PitchWheel = 0xE0
    };

public:
    ChannelMessage(double delay, uint8_t statusByte, uint8_t dataByte1, uint8_t dataByte2) :
    mDelay(delay), mStatusByte(statusByte), mDataByte1(dataByte1), mDataByte2(dataByte2) {}

    Type type() const {
        return static_cast<Type>(mStatusByte & 0xF0);
    }

    std::string typeString() const {
        switch (type()) {
            case Type::NoteOff:
                return "Note Off";
            case Type::NoteOn:
                return "Note On";
            case Type::PolyphonicAftertouch:
                return "Polyphonic Aftertouch";
            case Type::ControlChange:
                return "Control Change";
            case Type::ProgramChange:
                return "Program Change";
            case Type::ChannelAftertouch:
                return "Channel Aftertouch";
            case Type::PitchWheel:
                return "Pitch Wheel";
        }
        return "";
    }

    double delay() const {
        return mDelay;
    }

    uint8_t channel() const {
        return (mStatusByte & 0x0F) + 1;
    }

    uint8_t byte1() const {
        return mDataByte1;
    }

    uint8_t byte2() const {
        return mDataByte2;
    }

protected:
    const double mDelay;
    const uint8_t mStatusByte;
    const uint8_t mDataByte1;
    const uint8_t mDataByte2;
};

using MidiRecievedFunction = std::function<void (const ChannelMessage& channelMessage)>;

}