#include "InterruptCallback.h"

#include "InterruptDelegate.h"

namespace Emu8080
{
    InterruptCallback::InterruptCallback(const uint16_t address, InterruptDelegate * const delegate, const std::string &id)
    {
        this->address = address;
        this->delegate = delegate;
        this->id = id;
    }

    void InterruptCallback::SetAddress(const uint16_t address) { this->address = address; }
    void InterruptCallback::SetDelegate(InterruptDelegate * const delegate) { this->delegate = delegate; }
    void InterruptCallback::SetID(const std::string &id) { this->id = id; }

    const uint16_t InterruptCallback::GetAddress() const { return this->address; }
    InterruptDelegate * const InterruptCallback::GetDelegate() const { return this->delegate; }
    const std::string &InterruptCallback::GetID() const { return this->id; }

    void InterruptCallback::Invoke(Emulator * const emulator)
    {
        if (this->delegate != nullptr)
            this->delegate->HandleCallback(this, emulator);
    }
}