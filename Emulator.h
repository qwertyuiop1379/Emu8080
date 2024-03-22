#pragma once

#include <map>

#include "CPU.h"
#include "InterruptCallback.h"

namespace Emu8080
{
    class Emulator {
        private:
            CPU *cpu;

            std::string error;
            std::string output;
            std::string input;

            std::map<std::string, InterruptCallback *> interruptCallbacks;

        public:
            Emulator();
            ~Emulator();

            // CPU
            CPU * const GetCPU();
            void ResetState();

            // Run
            void Run();

            // Memory I/O
            void LoadMemoryFromROM(const char * const filename);
            void WriteMemory(const uint16_t address, const uint8_t * const bytes, const uint16_t size);
            void ReadMemory(const uint16_t address, uint8_t * const buffer, const uint16_t size);

            // Stream outputs
            const std::string GetErrorStream(bool clear = true);
            const std::string GetOutputStream(bool clear = true);

            // Stream inputs
            void SetInputStream(const std::string &stream);
            void AppendInputStream(const std::string &string);

            void SetOutputStream(const std::string &stream);
            void AppendOutputStream(const std::string &string);

            // Interrupt handlers
            void RegisterInterruptCallback(uint16_t address, InterruptDelegate * const delegate, const std::string &id);
            const InterruptCallback * const GetInterruptCallback(const std::string &id) const;
            void RemoveInterruptCallback(const std::string &id);
    };
}