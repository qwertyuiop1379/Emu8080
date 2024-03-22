#include <stdio.h>


#include "Emulator.h"
#include "InterruptDelegate.h"
#include "Util.h"

using namespace Emu8080;

class CPMOS : public InterruptDelegate {
    void HandleCallback(InterruptCallback * const callback, Emulator * const emulator) override
    {
        switch (callback->GetAddress()) {
            case 0x0: {
                emulator->GetCPU()->GetState()->SetHalt(true);
                break;
            }

            case 0x5: {
                auto cpu = emulator->GetCPU();
                uint8_t c = cpu->ReadRegister8(CPU::RegisterC);

                if (c == 9) {
                    uint16_t addr = cpu->ReadRegister16(CPU::RegisterPairDE);
                    std::string str;

                    while (true) {
                        uint8_t c = cpu->Read8(addr++);
                        if (c == '$')
                            break;
                        str += (char)c;
                    }

                    emulator->AppendOutputStream(str);
                } else if (c == 2) {
                    uint8_t e = cpu->ReadRegister8(CPU::RegisterE);
                    emulator->AppendOutputStream(std::string((const char *)(&e), 1));
                }

                break;
            }

            default: throw std::runtime_error(FormatString("Unknown interrupt vector $%x.\n", callback->GetAddress()));
        }
    }
};

int main(int argc, char **argv)
{
    Emulator *em = new Emulator();

    // Test ROM: https://github.com/ddelnano/8080-emulator/tree/master
    em->LoadMemoryFromROM("/Users/scoob/Downloads/TST8080.COM");

    // Emulate CP/M OS to handle interrupts
    CPMOS *os = new CPMOS();
    em->RegisterInterruptCallback(0x0, os, "reset");
    em->RegisterInterruptCallback(0x5, os, "msg");

    while (true) {
        em->Run();
        
        if (em->GetCPU()->GetState()->GetHalt())
            break;

        auto output = em->GetOutputStream();
        if (!output.empty())
            printf("%s", output.c_str());
    }

    printf("\n");

    delete em;
    return 0;
}