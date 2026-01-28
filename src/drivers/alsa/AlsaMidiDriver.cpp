#include "AlsaMidiDriver.h"
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <algorithm>

namespace nap {
namespace drivers {

class AlsaMidiDriver::Impl {
public:
    std::atomic<bool> initialized{false};
    std::atomic<bool> running{false};
    std::string clientName;
    int clientId = -1;

    std::vector<MidiPortInfo> ports;
    std::vector<std::string> connectedInputs;
    std::vector<std::string> connectedOutputs;
    std::vector<std::string> createdPorts;

    MidiCallback midiCallback;
    std::mutex callbackMutex;

    std::thread inputThread;
    std::atomic<bool> threadRunning{false};

    double ppq = 480.0;

    void inputThreadFunction() {
        while (threadRunning) {
            // Simulated MIDI input processing
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void populatePorts() {
        ports.clear();

        // Simulated MIDI ports
        MidiPortInfo virtualIn;
        virtualIn.id = "virtual:0:0";
        virtualIn.name = "Virtual Input";
        virtualIn.clientName = clientName;
        virtualIn.clientId = clientId;
        virtualIn.portId = 0;
        virtualIn.isInput = true;
        virtualIn.isOutput = false;
        virtualIn.isHardware = false;
        virtualIn.isVirtual = true;
        ports.push_back(virtualIn);

        MidiPortInfo virtualOut;
        virtualOut.id = "virtual:0:1";
        virtualOut.name = "Virtual Output";
        virtualOut.clientName = clientName;
        virtualOut.clientId = clientId;
        virtualOut.portId = 1;
        virtualOut.isInput = false;
        virtualOut.isOutput = true;
        virtualOut.isHardware = false;
        virtualOut.isVirtual = true;
        ports.push_back(virtualOut);

        MidiPortInfo hwMidi;
        hwMidi.id = "hw:0:0";
        hwMidi.name = "USB MIDI Interface";
        hwMidi.clientName = "USB MIDI";
        hwMidi.clientId = 20;
        hwMidi.portId = 0;
        hwMidi.isInput = true;
        hwMidi.isOutput = true;
        hwMidi.isHardware = true;
        hwMidi.isVirtual = false;
        ports.push_back(hwMidi);
    }
};

AlsaMidiDriver::AlsaMidiDriver() : pImpl(std::make_unique<Impl>()) {}

AlsaMidiDriver::~AlsaMidiDriver() {
    shutdown();
}

AlsaMidiDriver::AlsaMidiDriver(AlsaMidiDriver&&) noexcept = default;
AlsaMidiDriver& AlsaMidiDriver::operator=(AlsaMidiDriver&&) noexcept = default;

bool AlsaMidiDriver::initialize(const std::string& clientName) {
    if (pImpl->initialized) {
        return true;
    }

    pImpl->clientName = clientName;
    pImpl->clientId = 128; // Simulated client ID
    pImpl->populatePorts();
    pImpl->initialized = true;

    return true;
}

void AlsaMidiDriver::shutdown() {
    stop();
    pImpl->ports.clear();
    pImpl->connectedInputs.clear();
    pImpl->connectedOutputs.clear();
    pImpl->createdPorts.clear();
    pImpl->initialized = false;
}

bool AlsaMidiDriver::isInitialized() const {
    return pImpl->initialized;
}

std::vector<AlsaMidiDriver::MidiPortInfo> AlsaMidiDriver::enumeratePorts() const {
    return pImpl->ports;
}

std::vector<AlsaMidiDriver::MidiPortInfo> AlsaMidiDriver::getInputPorts() const {
    std::vector<MidiPortInfo> result;
    for (const auto& port : pImpl->ports) {
        if (port.isInput) {
            result.push_back(port);
        }
    }
    return result;
}

std::vector<AlsaMidiDriver::MidiPortInfo> AlsaMidiDriver::getOutputPorts() const {
    std::vector<MidiPortInfo> result;
    for (const auto& port : pImpl->ports) {
        if (port.isOutput) {
            result.push_back(port);
        }
    }
    return result;
}

bool AlsaMidiDriver::createInputPort(const std::string& name) {
    if (!pImpl->initialized) {
        return false;
    }

    MidiPortInfo port;
    port.id = "created:" + name;
    port.name = name;
    port.clientName = pImpl->clientName;
    port.clientId = pImpl->clientId;
    port.portId = static_cast<int>(pImpl->createdPorts.size());
    port.isInput = true;
    port.isOutput = false;
    port.isHardware = false;
    port.isVirtual = true;

    pImpl->ports.push_back(port);
    pImpl->createdPorts.push_back(name);

    return true;
}

bool AlsaMidiDriver::createOutputPort(const std::string& name) {
    if (!pImpl->initialized) {
        return false;
    }

    MidiPortInfo port;
    port.id = "created:" + name;
    port.name = name;
    port.clientName = pImpl->clientName;
    port.clientId = pImpl->clientId;
    port.portId = static_cast<int>(pImpl->createdPorts.size());
    port.isInput = false;
    port.isOutput = true;
    port.isHardware = false;
    port.isVirtual = true;

    pImpl->ports.push_back(port);
    pImpl->createdPorts.push_back(name);

    return true;
}

bool AlsaMidiDriver::deletePort(const std::string& portName) {
    auto it = std::find(pImpl->createdPorts.begin(), pImpl->createdPorts.end(), portName);
    if (it == pImpl->createdPorts.end()) {
        return false;
    }

    pImpl->createdPorts.erase(it);

    pImpl->ports.erase(
        std::remove_if(pImpl->ports.begin(), pImpl->ports.end(),
            [&portName](const MidiPortInfo& port) {
                return port.name == portName;
            }),
        pImpl->ports.end());

    return true;
}

bool AlsaMidiDriver::connectInput(const std::string& sourceName) {
    if (!pImpl->initialized) {
        return false;
    }
    pImpl->connectedInputs.push_back(sourceName);
    return true;
}

bool AlsaMidiDriver::connectOutput(const std::string& destName) {
    if (!pImpl->initialized) {
        return false;
    }
    pImpl->connectedOutputs.push_back(destName);
    return true;
}

bool AlsaMidiDriver::disconnectInput(const std::string& sourceName) {
    auto it = std::find(pImpl->connectedInputs.begin(),
                        pImpl->connectedInputs.end(), sourceName);
    if (it == pImpl->connectedInputs.end()) {
        return false;
    }
    pImpl->connectedInputs.erase(it);
    return true;
}

bool AlsaMidiDriver::disconnectOutput(const std::string& destName) {
    auto it = std::find(pImpl->connectedOutputs.begin(),
                        pImpl->connectedOutputs.end(), destName);
    if (it == pImpl->connectedOutputs.end()) {
        return false;
    }
    pImpl->connectedOutputs.erase(it);
    return true;
}

std::vector<std::string> AlsaMidiDriver::getConnectedInputs() const {
    return pImpl->connectedInputs;
}

std::vector<std::string> AlsaMidiDriver::getConnectedOutputs() const {
    return pImpl->connectedOutputs;
}

bool AlsaMidiDriver::start() {
    if (!pImpl->initialized || pImpl->running) {
        return pImpl->running;
    }

    pImpl->threadRunning = true;
    pImpl->running = true;
    pImpl->inputThread = std::thread(&Impl::inputThreadFunction, pImpl.get());

    return true;
}

void AlsaMidiDriver::stop() {
    if (!pImpl->running) {
        return;
    }

    pImpl->running = false;
    pImpl->threadRunning = false;

    if (pImpl->inputThread.joinable()) {
        pImpl->inputThread.join();
    }
}

bool AlsaMidiDriver::isRunning() const {
    return pImpl->running;
}

void AlsaMidiDriver::setMidiCallback(MidiCallback callback) {
    std::lock_guard<std::mutex> lock(pImpl->callbackMutex);
    pImpl->midiCallback = std::move(callback);
}

bool AlsaMidiDriver::sendEvent(const MidiEvent& event) {
    if (!pImpl->running) {
        return false;
    }
    // Simulated event sending
    return true;
}

bool AlsaMidiDriver::sendNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
    MidiEvent event;
    event.type = MidiEvent::Type::NoteOn;
    event.channel = channel;
    event.data1 = note;
    event.data2 = velocity;
    event.timestamp = getCurrentTime();
    return sendEvent(event);
}

bool AlsaMidiDriver::sendNoteOff(uint8_t channel, uint8_t note, uint8_t velocity) {
    MidiEvent event;
    event.type = MidiEvent::Type::NoteOff;
    event.channel = channel;
    event.data1 = note;
    event.data2 = velocity;
    event.timestamp = getCurrentTime();
    return sendEvent(event);
}

bool AlsaMidiDriver::sendControlChange(uint8_t channel, uint8_t controller, uint8_t value) {
    MidiEvent event;
    event.type = MidiEvent::Type::ControlChange;
    event.channel = channel;
    event.data1 = controller;
    event.data2 = value;
    event.timestamp = getCurrentTime();
    return sendEvent(event);
}

bool AlsaMidiDriver::sendProgramChange(uint8_t channel, uint8_t program) {
    MidiEvent event;
    event.type = MidiEvent::Type::ProgramChange;
    event.channel = channel;
    event.data1 = program;
    event.data2 = 0;
    event.timestamp = getCurrentTime();
    return sendEvent(event);
}

bool AlsaMidiDriver::sendPitchBend(uint8_t channel, int16_t value) {
    MidiEvent event;
    event.type = MidiEvent::Type::PitchBend;
    event.channel = channel;
    event.data1 = static_cast<uint8_t>(value & 0x7F);
    event.data2 = static_cast<uint8_t>((value >> 7) & 0x7F);
    event.timestamp = getCurrentTime();
    return sendEvent(event);
}

bool AlsaMidiDriver::sendSysEx(const std::vector<uint8_t>& data) {
    MidiEvent event;
    event.type = MidiEvent::Type::SysEx;
    event.channel = 0;
    event.data1 = 0;
    event.data2 = 0;
    event.timestamp = getCurrentTime();
    event.sysexData = data;
    return sendEvent(event);
}

uint64_t AlsaMidiDriver::getCurrentTime() const {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(
        now.time_since_epoch()).count();
}

void AlsaMidiDriver::setTimebase(double ppq) {
    pImpl->ppq = ppq;
}

bool AlsaMidiDriver::createVirtualInputPort(const std::string& name) {
    return createInputPort(name);
}

bool AlsaMidiDriver::createVirtualOutputPort(const std::string& name) {
    return createOutputPort(name);
}

} // namespace drivers
} // namespace nap
