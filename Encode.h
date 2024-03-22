#pragma once

#include <string>

namespace Emu8080 {
    class Encode {
        public:
            // Data structures
            enum class ArgumentFormat {
                InstructionOnly,
                OneRegister8,
                TwoRegister8s,
                OneRegister16,
                OneRegister16Restricted,
                OneRegister16PSWAllowed,
                OneImmediate8,
                OneImmediate8Restricted,
                OneImmediate16,
                OneVector,
                OnePort,
                OneRegister8OneImmediate8,
                OneRegister16OneImmediate16,
                OneConditionCode,
                OneConditionCodeOneImmediate16
            };

            // Instruction encoding
            static void EncodeInstruction(const std::string &str, uint8_t * const buffer, uint8_t *size = nullptr);
            static std::string DecodeInstruction(const uint8_t * const bytes, uint8_t *size = nullptr);

            // Condition code encoding
            static uint8_t IsValidConditionCode(const std::string &_str);
            static uint8_t ReadConditionCode(const std::string &_str);

            // Instruction arguments
            static ArgumentFormat DetermineFormat(const std::string &instr);
            static bool ExtractArguments(const std::string &_str, ArgumentFormat format, uint16_t *arg1, uint16_t *arg2);
    };
}