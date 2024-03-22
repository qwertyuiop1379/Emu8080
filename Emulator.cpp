#include "Emulator.h"

#include <stdexcept>
#include "Util.h"

#define DEBUG 0

namespace Emu8080
{
    void logfunc(const std::string &message)
    {
        if (DEBUG)
            printf("%s\n", message.c_str());
    }

    Emulator::Emulator()
    {
        this->cpu = new CPU(logfunc, 0x10000);
        this->ResetState();
    }

    Emulator::~Emulator()
    {
        for (auto pair : this->interruptCallbacks)
            delete pair.second;

        delete this->cpu;
    }

    CPU * const Emulator::GetCPU() { return this->cpu; }
    
    void Emulator::ResetState()
    {
        CPUState state;
        state.SetMemorySize(0x10000);
        state.SetPC(0x100);
        state.CopyTo(this->cpu->GetState());

        // for now
        this->cpu->Write8(0x5, 0xC9);
        this->cpu->WritePC(0x100);
    }

    void Emulator::Run()
    {
        auto pc = this->cpu->ReadPC();

        if (this->cpu->GetState()->GetWaitCycles() == 0) {
            for (auto pair : this->interruptCallbacks) {
                if (pair.second->GetAddress() == pc)
                    pair.second->Invoke(this);
            }
        }

        this->cpu->ExecuteCycle();
    }

    void Emulator::LoadMemoryFromROM(const char * const filename)
    {
        FILE *rom = fopen(filename, "rb");

        if (rom == nullptr)
            throw std::runtime_error(FormatString("Failed to open file '%s'.", filename));

        fseek(rom, 0, SEEK_END);
        size_t size = ftell(rom);
        
        void *buffer = malloc(size);

        fseek(rom, 0, SEEK_SET);
        fread(buffer, 1, size, rom);
        fclose(rom);

        this->cpu->WriteBytes(0x100, (uint8_t *)buffer, size);
        free(buffer);
    }

    void Emulator::WriteMemory(const uint16_t address, const uint8_t * const bytes, const uint16_t size)
    {
        this->cpu->WriteBytes(address, bytes, size);
    }

    void Emulator::ReadMemory(const uint16_t address, uint8_t * const buffer, const uint16_t size)
    {
        this->cpu->ReadBytes(address, buffer, size);
    }

    const std::string Emulator::GetErrorStream(bool clear)
    {
        auto str = this->error;

        if (clear == true)
            this->error.clear();

        return str;
    }

    const std::string Emulator::GetOutputStream(bool clear)
    {
        auto str = this->output;

        if (clear == true)
            this->output.clear();
        
        return str;
    }

    void Emulator::SetInputStream(const std::string &stream)
    {
        this->input = stream;
    }

    void Emulator::AppendInputStream(const std::string &string)
    {
        this->input += string;
    }

    void Emulator::SetOutputStream(const std::string &stream)
    {
        this->output = stream;
    }

    void Emulator::AppendOutputStream(const std::string &string)
    {
        this->output += string;
    }

    void Emulator::RegisterInterruptCallback(uint16_t address, InterruptDelegate * const delegate, const std::string &id)
    {
        if (delegate == nullptr)
            throw std::runtime_error("InterruptDelegate must not be null.");
        if (this->interruptCallbacks.find(id) != this->interruptCallbacks.end())
            throw std::runtime_error(FormatString("Interrupt handler '%s' is already registered.", id.c_str()));

        InterruptCallback *handler = new InterruptCallback(address, delegate, id);
        this->interruptCallbacks[id] = handler;
    }

    const InterruptCallback * const Emulator::GetInterruptCallback(const std::string &id) const
    {
        auto callback = this->interruptCallbacks.find(id);
        return callback == this->interruptCallbacks.end() ? callback->second : nullptr;
    }

    void Emulator::RemoveInterruptCallback(const std::string &id)
    {
        for (auto pair : this->interruptCallbacks) {
            if (pair.first == id)
                this->interruptCallbacks.erase(id);
        }
    }
}