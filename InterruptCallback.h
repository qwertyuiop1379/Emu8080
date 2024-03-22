#pragma once

#include <string>
#include <stdint.h>

namespace Emu8080
{
    class InterruptDelegate;
    class Emulator;

    class InterruptCallback {
        private:
            uint16_t address;
            std::string id;
            InterruptDelegate *delegate;

        public:
            InterruptCallback(const uint16_t address, InterruptDelegate * const delegate, const std::string &id);

            void SetAddress(const uint16_t address);
            void SetDelegate(InterruptDelegate * const delegate);
            void SetID(const std::string &id);

            const uint16_t GetAddress() const;
            InterruptDelegate * const GetDelegate() const;
            const std::string &GetID() const;

            void Invoke(Emulator * const emulator);
    };
}