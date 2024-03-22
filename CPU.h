#pragma once

#include <stdint.h>
#include <string>

#include "CPUState.h"

namespace Emu8080
{
    class CPU {
        private:
            void (*logFunction)(const std::string &);
            CPUState *state;

        public:
            // Constants
            static const uint8_t RegisterA = 0b111;
            static const uint8_t RegisterB = 0b000;
            static const uint8_t RegisterC = 0b001;
            static const uint8_t RegisterD = 0b010;
            static const uint8_t RegisterE = 0b011;
            static const uint8_t RegisterH = 0b100;
            static const uint8_t RegisterL = 0b101;
            static const uint8_t RegisterM = 0b110;

            static const uint8_t RegisterPairBC = 0b00;
            static const uint8_t RegisterPairDE = 0b01;
            static const uint8_t RegisterPairHL = 0b10;
            static const uint8_t RegisterPairSP = 0b11;
            static const uint8_t RegisterPairPSW = 0b11;

            // Enumerations
            enum class Flag { S = 7, Z = 6, A = 4, P = 2, C = 0 };

            // Constructor/destructor
            CPU(void (*logFunction)(const std::string &), uint32_t memorySize);
            ~CPU();

            // Assertions
            void AssertValidAddress(uint16_t addr) const;
            void AssertValidAddressRange(uint16_t addr, uint16_t size) const;

            // State
            CPUState * const GetState();
            void SetState(const CPUState * const state);

            // Log
            template<typename ... Args> void Log(const std::string &format, Args ... args) const;

            // Memory write
            void Write8(uint16_t addr, uint8_t value);
            void Write16(uint16_t addr, uint16_t value);
            void WriteBytes(uint16_t addr, const uint8_t * const bytes, uint16_t size);

            // Memory read
            uint8_t Read8(uint16_t addr) const;
            uint16_t Read16(uint16_t addr) const;
            void ReadBytes(uint16_t addr, void * const buffer, uint16_t size) const;

            // Register write
            void WritePC(uint16_t pc);
            void WriteSP(uint16_t sp);
            void WriteRegister8(uint8_t r, uint8_t value);
            void WriteRegister16(uint8_t r, uint16_t value, bool spAvailable = true);

            // Register read
            uint16_t ReadPC() const;
            uint16_t ReadSP() const;
            uint8_t ReadRegister8(uint8_t r) const;
            uint16_t ReadRegister16(uint8_t r, bool spAvailable = true) const;
            
            // Flags
            void SetFlag(Flag f, bool value);
            bool GetFlag(Flag f) const;
            void CalculateSZP(uint8_t n);
            bool ConditionMet(uint8_t condition) const;

            // Execution
            void ExecuteCycle();
            uint8_t ExecuteInstruction();

            // Stack
            void Push(uint16_t addr);
            uint16_t Pop();

            // Control flow
            void Call(uint16_t addr);
            void Return();
            bool GetInteruptsEnabled() const;

            // I/O
            void OutputData(uint8_t port, uint8_t data) const;
            uint8_t InputData(uint8_t port);

            // Arithmetic
            void Arithmetic(uint8_t opcode, uint8_t value);
            void Add(uint8_t value);
            void Adc(uint8_t value);
            void Sub(uint8_t value);
            void Sbc(uint8_t value);
            void And(uint8_t value);
            void Or(uint8_t value);
            void Xor(uint8_t value);
            void Cmp(uint8_t value);
    };

    // Enum constructors
    CPU::Flag MakeFlag(uint8_t f);

    // Enum string functions
    const char *StringForRegister8(uint8_t r);
    const char *StringForRegister16(uint8_t r, bool spAvailable = true);
    const char *StringForFlag(CPU::Flag f);

    uint8_t Register8ForString(const std::string &str);
    uint8_t Register16ForString(const std::string &str);
    CPU::Flag FlagForString(const std::string &str);
}