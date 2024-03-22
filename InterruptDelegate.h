#pragma once

namespace Emu8080
{
    class InterruptCallback;
    class Emulator;
    
    class InterruptDelegate {
        public:
            virtual void HandleCallback(InterruptCallback * const callback, Emulator * const emulator) = 0;
    };
}