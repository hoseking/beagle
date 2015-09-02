//  Copyright (c) 2015 hoseking. All rights reserved.

#pragma once

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace midi {

typedef unsigned char byte;

class ChannelMessage {
public:
    enum class Type : byte {
        NoteOff = 0x80,
        NoteOn = 0x90,
        PolyphonicAftertouch = 0xA0,
        ControlChange = 0xB0,
        ProgramChange = 0xC0,
        ChannelAftertouch = 0xD0,
        PitchWheel = 0xE0
    };

public:
    ChannelMessage(const byte& statusByte, const byte& dataByte1, const byte& dataByte2) :
    mStatusByte(statusByte), mDataByte1(dataByte1), mDataByte2(dataByte2) {}

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

    std::vector<unsigned char> message() const {
        std::vector<unsigned char> message{
            statusByte(),
            byte1(),
            byte2()
        };
        return message;
    }

    byte channel() const {
        return (mStatusByte & 0x0F) + 1;
    }

    byte statusByte() const {
        return mStatusByte;
    }

    byte byte1() const {
        return mDataByte1;
    }

    byte byte2() const {
        return mDataByte2;
    }

protected:
    const byte mStatusByte;
    const byte mDataByte1;
    const byte mDataByte2;
};

class SysExMessage {
public:
    void addByte(const unsigned char& byte) {
        mBytes.push_back(byte);
    }

    std::vector<unsigned char> message() const {
        std::vector<unsigned char> message;
        message.push_back(0xf0);
        for (auto& byte : mBytes)
            message.push_back(byte);
        message.push_back(0xf7);
        return message;
    };

private:
    std::vector<unsigned char> mBytes;
};

using MidiRecievedFunction = std::function<void (const ChannelMessage& channelMessage, const double& delay)>;

}
