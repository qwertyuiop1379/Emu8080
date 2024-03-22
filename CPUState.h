#pragma once

#include <stdint.h>

namespace Emu8080
{
    class CPUState {
        private:
            uint8_t *memory;
            uint32_t memorySize;

            uint16_t pc;
            uint16_t sp;

            uint8_t registers[7];

            uint8_t flags;

            uint8_t waitCycles;

            bool halt;
            bool interuptsEnabled;

        public:
            CPUState();
            ~CPUState();

            bool IsEqual(const CPUState * const state, bool compareRAM = true);
            void CopyTo(CPUState * const state, bool copyMemory = true) const;
            
            const uint8_t *GetMemory() const;
            uint32_t GetMemorySize() const;

            void SetMemory(const uint8_t * const memory, uint32_t size);
            void SetMemorySize(uint32_t size);
            void WriteByte(uint16_t address, uint8_t value);
            void WriteBytes(uint16_t address, const uint8_t * const bytes, uint32_t size);

            uint16_t GetPC() const;
            uint16_t GetSP() const;

            void SetPC(uint16_t pc);
            void SetSP(uint16_t sp);

            uint8_t GetRegister(uint8_t index) const;
            void SetRegister(uint8_t index, uint8_t value);

            uint8_t GetFlags() const;
            void SetFlags(const uint8_t flags);

            uint8_t GetWaitCycles() const;
            void SetWaitCycles(const uint8_t count);

            bool GetHalt() const;
            bool GetInteruptsEnabled() const;

            void SetHalt(bool halt);
            void SetInterruptsEnabled(bool enabled);
    };
}