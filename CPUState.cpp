#include "CPUState.h"

#include <stdlib.h>
#include <memory>

namespace Emu8080
{
    CPUState::CPUState()
    {
        this->memory = nullptr;
        this->memorySize = 0;

        this->halt = false;
        this->interuptsEnabled = false;

        this->flags = 0x2;

        this->waitCycles = 0;
    }

    CPUState::~CPUState()
    {
        if (this->memory != nullptr)
            free((void *)this->memory);
    }

    bool CPUState::IsEqual(const CPUState * const state, bool compareRAM)
    {
        if (this->memorySize != state->memorySize)
            return false;
        if (this->pc != state->pc)
            return false;
        if (this->sp != state->sp)
            return false;
        if (this->flags != state->flags)
            return false;
        if (this->halt != state->halt)
            return false;
        if (this->interuptsEnabled != state->interuptsEnabled)
            return false;

        for (int i = 0; i < 7; i++) {
            if (this->registers[i] != state->registers[i])
                return false;
        }

        if (compareRAM) {
            if (this->memory == nullptr || state->memory == nullptr)
                return false;
            if (std::memcmp(this->memory, state->memory, this->memorySize) != 0)
                return false;
        }

        return true;
    }

    void CPUState::CopyTo(CPUState * const state, bool copyMemory) const
    {
        if (copyMemory && this->memorySize > 0) {
            if (state->memory != nullptr)
                free(state->memory);

            state->memory = (uint8_t *)malloc(this->memorySize);
            state->memorySize = this->memorySize;

            memcpy(state->memory, this->memory, this->memorySize);
        } else {
            state->memory = nullptr;
            state->memorySize = 0;
        }

        state->pc = this->pc;
        state->sp = this->sp;
        state->flags = this->flags;
        state->halt = this->halt;
        state->interuptsEnabled = this->interuptsEnabled;

        for (int i = 0; i < 7; i++)
            state->registers[i] = this->registers[i];
    }
    
    const uint8_t *CPUState::GetMemory() const
    {
        return this->memory;
    }

    uint32_t CPUState::GetMemorySize() const
    {
        return this->memorySize;
    }

    void CPUState::SetMemory(const uint8_t * const memory, const uint32_t size)
    {
        if (this->memory != nullptr)
            free(this->memory);

        this->memorySize = size;
        this->memory = (uint8_t *)malloc(size);

        memcpy(this->memory, memory, size);
    }

    void CPUState::SetMemorySize(uint32_t size)
    {
        this->memorySize = size;

        if (size == 0) {
            if (this->memory != nullptr) {
                free(this->memory);
                this->memory = nullptr;
            }

            return;
        } else {
            if (this->memory == nullptr)
                this->memory = (uint8_t *)malloc(size);
            else
                this->memory = (uint8_t *)realloc(this->memory, size);
        }
    }

    void CPUState::WriteByte(uint16_t address, uint8_t value)
    {
        this->memory[address] = value;
    }

    void CPUState::WriteBytes(uint16_t address, const uint8_t * const bytes, uint32_t size)
    {
        memcpy(this->memory + address, bytes, size);
    }

    uint16_t CPUState::GetPC() const
    {
        return this->pc;
    }

    uint16_t CPUState::GetSP() const
    {
        return this->sp;
    }

    void CPUState::SetPC(uint16_t pc)
    {
        this->pc = pc;
    }

    void CPUState::SetSP(uint16_t sp)
    {
        this->sp = sp;
    }

    uint8_t CPUState::GetRegister(uint8_t index) const
    {
        return this->registers[index];
    }

    void CPUState::SetRegister(uint8_t index, uint8_t value)
    {
        this->registers[index] = value;
    }

    uint8_t CPUState::GetFlags() const
    {
        return this->flags;
    }

    void CPUState::SetFlags(const uint8_t flags)
    {
        this->flags = flags;
    }

    uint8_t CPUState::GetWaitCycles() const
    {
        return this->waitCycles;
    }

    void CPUState::SetWaitCycles(const uint8_t count)
    {
        this->waitCycles = count;
    }

    bool CPUState::GetHalt() const
    {
        return this->halt;
    }

    bool CPUState::GetInteruptsEnabled() const
    {
        return this->interuptsEnabled;
    }

    void CPUState::SetHalt(bool halt)
    {
        this->halt = halt;
    }

    void CPUState::SetInterruptsEnabled(bool enabled)
    {
        this->interuptsEnabled = enabled;
    }
}