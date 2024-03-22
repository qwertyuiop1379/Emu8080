#include "CPU.h"

#include <stdexcept>
#include "Util.h"
#include "Encode.h"

namespace Emu8080
{
    CPU::Flag MakeFlag(uint8_t f)
    {
        if (f > 7 || f == 5 || f == 3 || f == 1)
            throw std::runtime_error(FormatString("Malformed CPU::Flag value %d.", f));

        return (CPU::Flag)f;
    }

    const char *StringForRegister8(uint8_t r)
    {
        switch (r) {
            case 0b111: return "a";
            case 0b000: return "b";
            case 0b001: return "c";
            case 0b010: return "d";
            case 0b011: return "e";
            case 0b100: return "h";
            case 0b101: return "l";
            case 0b110: return "m";
        }

        throw std::runtime_error(FormatString("Malformed register value %d.", r));
    }

    const char *StringForRegister16(uint8_t r, bool spAvailable)
    {
        switch (r) {
            case 0b00: return "bc";
            case 0b01: return "de";
            case 0b10: return "hl";
            case 0b11: return spAvailable ? "sp" : "psw";
        }

        throw std::runtime_error(FormatString("Malformed register pair value.", r));
    }

    const char *StringForFlag(CPU::Flag f)
    {
        switch (f) {
            case CPU::Flag::S: return "s";
            case CPU::Flag::Z: return "z";
            case CPU::Flag::P: return "p";
            case CPU::Flag::C: return "c";
            case CPU::Flag::A: return "a";
        }

        throw std::runtime_error(FormatString("Malformed CPU::Flag value %d.", f));
    }

    uint8_t Register8ForString(const std::string &_str)
    {
        auto str = LowercaseString(_str);

        switch (*str.c_str()) {
            case 'a': return CPU::RegisterA;
            case 'b': return CPU::RegisterB;
            case 'c': return CPU::RegisterC;
            case 'd': return CPU::RegisterD;
            case 'e': return CPU::RegisterE;
            case 'h': return CPU::RegisterH;
            case 'l': return CPU::RegisterL;
            case 'm': return CPU::RegisterM;
            default: throw std::runtime_error(FormatString("Unknown register '%s'.", str.c_str()));
        }
    }

    uint8_t Register16ForString(const std::string &_str)
    {
        auto str = LowercaseString(_str);
        
        if (str == "bc" || str == "b")
            return CPU::RegisterPairBC;
        else if (str == "de" || str == "d")
            return CPU::RegisterPairDE;
        else if (str == "hl" || str == "h")
            return CPU::RegisterPairHL;
        else if (str == "sp")
            return CPU::RegisterPairSP;
        else
            throw std::runtime_error(FormatString("Unknown register '%s'.", str.c_str()));
    }

    CPU::Flag FlagForString(const std::string &_str)
    {
        if (_str.length() != 1)
            throw std::runtime_error("Flag string length must be 1.");

        auto str = LowercaseString(_str);

        switch (*str.c_str()) {
            case 's': return CPU::Flag::S;
            case 'z': return CPU::Flag::Z;
            case 'a': return CPU::Flag::A;
            case 'p': return CPU::Flag::P;
            case 'c': return CPU::Flag::C;
            default: throw std::runtime_error(FormatString("Unknown flag '%s'.", str.c_str()));
        }
    }

    CPU::CPU(void (*logFunction)(const std::string &), uint32_t memorySize)
    {
        this->logFunction = logFunction;

        this->state = new CPUState();
        this->state->SetMemorySize(memorySize);

        this->Log("Initialized CPU.");
    }

    CPU::~CPU()
    {
        delete this->state;
        this->Log("Destructed CPU.");
    }

    void CPU::AssertValidAddress(uint16_t addr) const
    {
        if (addr >= this->state->GetMemorySize())
            throw std::runtime_error(FormatString("Address 0x%x exceeds memory size (0x%x).", addr, this->state->GetMemorySize()));
    }

    void CPU::AssertValidAddressRange(uint16_t addrStart, uint16_t addrEnd) const
    {
        if (addrEnd >= this->state->GetMemorySize())
            throw std::runtime_error(FormatString("Address range 0x%x-0x%x exceeds memory size (0x%x).", addrStart, addrEnd, this->state->GetMemorySize()));
    }

    CPUState * const CPU::GetState() { return this->state; }
    void CPU::SetState(const CPUState * const state) { state->CopyTo(this->state); }

    template<typename ... Args>
    void CPU::Log(const std::string &format, Args ... args) const
    {
        if (this->logFunction != nullptr)
            this->logFunction(FormatString("[CPU] %s", FormatString(format, args ...).c_str()));
    }

    void CPU::Write8(uint16_t addr, uint8_t value)
    {
        this->AssertValidAddress(addr);
        this->state->WriteByte(addr, value);

        this->Log("Wrote 0x%02x to addr 0x%04x.", value, addr);
    }

    void CPU::Write16(uint16_t addr, uint16_t value)
    {
        this->AssertValidAddressRange(addr, addr + 1);
        this->state->WriteByte(addr, value & 0xFF);
        this->state->WriteByte(addr + 1, value >> 8);

        this->Log("Wrote 0x%04x to addr 0x%04x.", value, addr);
    }

    void CPU::WriteBytes(uint16_t addr, const uint8_t * const bytes, uint16_t size)
    {
        this->AssertValidAddressRange(addr, addr + size);
        this->state->WriteBytes(addr, bytes, size);

        this->Log("Write 0x%x bytes to addr 0x%04x.", size, addr);
    }

    uint8_t CPU::Read8(uint16_t addr) const
    {
        this->AssertValidAddress(addr);
        auto value = this->state->GetMemory()[addr];

        this->Log("Read 0x%02x from addr 0x%04x.", value, addr);
        return value;
    }

    uint16_t CPU::Read16(uint16_t addr) const
    {
        this->AssertValidAddressRange(addr, addr + 1);
        auto value = this->state->GetMemory()[addr] | (this->state->GetMemory()[addr + 1] << 8);

        this->Log("Read 0x%04x from addr 0x%04x.", value, addr);
        return value;
    }

    void CPU::ReadBytes(uint16_t addr, void * const buffer, uint16_t size) const
    {
        this->AssertValidAddressRange(addr, addr + size);
        std::memcpy(buffer, this->state->GetMemory() + addr, size);

        this->Log("Read 0x%x bytes from addr 0x%04x.", size, addr);
    }

    void CPU::WritePC(uint16_t pc)
    {
        this->state->SetPC(pc);

        this->Log("Wrote 0x%04x to PC.", pc);
    }

    void CPU::WriteSP(uint16_t sp)
    {
        this->state->SetSP(sp);

        this->Log("Wrote 0x%04x to SP.", sp);
    }

    void CPU::WriteRegister8(uint8_t r, uint8_t value)
    {
        if (r == 0b110) {
            uint16_t addr = this->ReadRegister16(CPU::RegisterPairHL);
            this->Write8(addr, value);
        } else {
            this->state->SetRegister((r + 1) & 0b111, value);
        }

        this->Log("Wrote 0x%02x to register %s.", value, StringForRegister8(r));
    }

    void CPU::WriteRegister16(uint8_t r, uint16_t value, bool spAvailable)
    {
        uint8_t lo = value & 0xFF;
        uint8_t hi = value >> 8;

        switch (r) {
            case 0b00: this->WriteRegister8(CPU::RegisterB, hi); this->WriteRegister8(CPU::RegisterC, lo); break;
            case 0b01: this->WriteRegister8(CPU::RegisterD, hi); this->WriteRegister8(CPU::RegisterE, lo); break;
            case 0b10: this->WriteRegister8(CPU::RegisterH, hi); this->WriteRegister8(CPU::RegisterL, lo); break;
            case 0b11: {
                if (spAvailable) {
                    this->state->SetSP(value);
                } else {
                    this->state->SetFlags(lo);
                    this->WriteRegister8(CPU::RegisterA, hi);
                    this->Log("Wrote 0x%x to flags register.", this->state->GetFlags());
                }

                break;
            }
        }
        
        this->Log("Wrote 0x%04x to register pair %s.", value, StringForRegister16(r, spAvailable));
    }

    uint16_t CPU::ReadPC() const
    {
        this->Log("Read 0x%04x from PC.", this->state->GetPC());
        return this->state->GetPC();
    }

    uint16_t CPU::ReadSP() const
    {
        this->Log("Read 0x%04x from SP.", this->state->GetSP());
        return this->state->GetSP();
    }

    uint8_t CPU::ReadRegister8(uint8_t r) const
    {
        uint8_t value;

        if (r == 0b110) {
            uint16_t addr = this->ReadRegister16(CPU::RegisterPairHL);
            value = this->Read8(addr);
        } else {
            value = this->state->GetRegister((r + 1) & 0b111);
        }

        this->Log("Read 0x%02x from register %s.", value, StringForRegister8(r));
        return value;
    }

    uint16_t CPU::ReadRegister16(uint8_t r, bool spAvailable) const
    {
        uint16_t value;
        uint8_t lo;
        uint8_t hi;

        switch (r) {
            case 0b00: hi = this->ReadRegister8(CPU::RegisterB); lo = this->ReadRegister8(CPU::RegisterC); break;
            case 0b01: hi = this->ReadRegister8(CPU::RegisterD); lo = this->ReadRegister8(CPU::RegisterE); break;
            case 0b10: hi = this->ReadRegister8(CPU::RegisterH); lo = this->ReadRegister8(CPU::RegisterL); break;
            case 0b11: {
                if (spAvailable) {
                    value = this->state->GetSP();
                } else {
                    this->Log("Read 0x%x from flags register.", this->state->GetFlags());
                    value = (this->ReadRegister8(CPU::RegisterA) << 8) | this->state->GetFlags();
                }

                break;
            }
        }

        if (r != 0b11)
            value = (hi << 8) | lo;

        this->Log("Read 0x%04x from register pair %s.", value, StringForRegister16(r));
        return value;
    }

    void CPU::SetFlag(Flag f, bool value)
    {
        uint8_t n = (uint8_t)f;
        this->state->SetFlags((this->state->GetFlags() & ~((uint8_t)1 << n)) | ((uint8_t)value << n));
        
        this->Log("Set flag %s to %d.", StringForFlag(f), value);
    }

    bool CPU::GetFlag(Flag f) const
    {
        uint8_t n = (uint8_t)f;
        bool value = (this->state->GetFlags() >> n) & 1;

        this->Log("Read %d from flag %s.", value, StringForFlag(f));
        return value;
    }

    void CPU::CalculateSZP(uint8_t n)
    {
        this->SetFlag(CPU::Flag::S, n >> 7);
        this->SetFlag(CPU::Flag::Z, n == 0);

        uint8_t ones = 0;
        for (int i = 0; i < 8; i++) {
            if ((n >> i) & 1)
                ones++;
        }

        this->SetFlag(CPU::Flag::P, (ones % 2) == 0);
    }

    bool CPU::ConditionMet(uint8_t condition) const
    {
        switch (condition) {
            case 0b000: return this->GetFlag(CPU::Flag::Z) == 0;
            case 0b001: return this->GetFlag(CPU::Flag::Z) != 0;
            case 0b010: return this->GetFlag(CPU::Flag::C) == 0;
            case 0b011: return this->GetFlag(CPU::Flag::C) != 0;
            case 0b100: return this->GetFlag(CPU::Flag::P) == 0;
            case 0b101: return this->GetFlag(CPU::Flag::P) != 0;
            case 0b110: return this->GetFlag(CPU::Flag::S) == 0;
            case 0b111: return this->GetFlag(CPU::Flag::S) != 0;
        }

        throw std::runtime_error(FormatString("Malformed condition code 0x%x.", condition));
    }

    void CPU::ExecuteCycle()
    {
        if (this->GetState()->GetHalt() == true)
            return;

        uint8_t wait = this->state->GetWaitCycles();

        if (wait > 0) {
            this->state->SetWaitCycles(wait - 1);
            return;
        }

        this->state->SetWaitCycles(this->ExecuteInstruction());
    }

    uint8_t CPU::ExecuteInstruction()
    {
        uint16_t pc = this->ReadPC();
        this->WritePC(pc + 1);

        uint8_t instruction = this->Read8(pc);
        this->Log("Executing instruction 0x%02x: %s.", instruction, Encode::DecodeInstruction(this->state->GetMemory() + pc).c_str());

        uint8_t field = ExtractBits8(instruction, 7, 2);

        if (field == 0b00) {
            uint8_t opcode = ExtractBits8(instruction, 1, 3);
            if (opcode == 0b000) {
                // nop
                return 4;
            }

            if (opcode == 0b110) {
                // mvi d, #imm
                
                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 1);

                uint8_t imm = this->Read8(pc);
                uint8_t dest = ExtractBits8(instruction, 4, 3);

                this->WriteRegister8(dest, imm);
                return dest == 0b110 ? 10 : 7;
            }

            if (opcode == 0b111) {
                uint8_t opcode = ExtractBits8(instruction, 4, 3);
                
                switch (opcode)
                {
                    case 0b000: {
                        // rlc

                        uint8_t a = this->ReadRegister8(CPU::RegisterA);

                        this->SetFlag(CPU::Flag::C, a >> 7);
                        this->WriteRegister8(CPU::RegisterA, (a << 1) | (a >> 7));
                        
                        break;
                    }

                    case 0b001: {
                        // rrc
                        
                        uint8_t a = this->ReadRegister8(CPU::RegisterA);

                        this->SetFlag(CPU::Flag::C, a & 1);
                        this->WriteRegister8(CPU::RegisterA, (a >> 1) | ((a & 1) << 7));

                        break;
                    }

                    case 0b010: {
                        // ral

                        bool carry = this->GetFlag(CPU::Flag::C);
                        uint8_t a = this->ReadRegister8(CPU::RegisterA);

                        this->SetFlag(CPU::Flag::C, a >> 7);
                        this->WriteRegister8(CPU::RegisterA, (a << 1) | carry);

                        break;
                    }

                    case 0b011: {
                        // rar

                        bool carry = this->GetFlag(CPU::Flag::C);
                        uint8_t a = this->ReadRegister8(CPU::RegisterA);

                        this->SetFlag(CPU::Flag::C, a & 1);
                        this->WriteRegister8(CPU::RegisterA, (a >> 1) | (carry << 7));

                        break;
                    }

                    case 0b100: {
                        // daa

                        uint8_t a = this->ReadRegister8(CPU::RegisterA);

                        if ((a & 0xF) > 0x9 || this->GetFlag(CPU::Flag::A)) {
                            this->SetFlag(CPU::Flag::A, (a & 0xF) + 0x6 >= 0x10);
                            a += 0x6;
                        }

                        if ((a >> 4) > 0x9 || this->GetFlag(CPU::Flag::C)) {
                            if ((a >> 4) + 0x6 >= 0x10)
                                this->SetFlag(CPU::Flag::C, 1);
                            a += 0x60;
                        }

                        this->WriteRegister8(CPU::RegisterA, a);
                        this->CalculateSZP(a);

                        break;
                    }

                    case 0b101: {
                        // cma

                        uint8_t a = this->ReadRegister8(CPU::RegisterA);
                        this->WriteRegister8(CPU::RegisterA, ~a);

                        break;
                    }

                    case 0b110: {
                        // stc

                        this->SetFlag(CPU::Flag::C, 1);

                        break;
                    }

                    case 0b111: {
                        // cmc

                        bool c = this->GetFlag(CPU::Flag::C);
                        this->SetFlag(CPU::Flag::C, !c);

                        break;
                    }
                }

                return 4;
            }

            if (opcode == 0b010) {
                uint8_t rp = ExtractBits8(instruction, 5, 2);

                if (ExtractBits8(instruction, 4, 1) == 0b1) {
                    switch (rp) {
                        case 0b11: {
                            // lda addr

                            uint16_t pc = this->ReadPC();
                            this->WritePC(pc + 2);

                            uint16_t addr = this->Read16(pc);
                            uint8_t value = this->Read8(addr);

                            this->WriteRegister8(CPU::RegisterA, value);

                            return 13;
                        }

                        case 0b10: {
                            // lhld addr

                            uint16_t pc = this->ReadPC();
                            this->WritePC(pc + 2);

                            uint16_t addr = this->Read16(pc);
                            uint16_t value = this->Read16(addr);

                            this->WriteRegister16(CPU::RegisterPairHL, value);

                            return 16;
                        }

                        default: {
                            // ldax rp
                            
                            uint16_t addr = this->ReadRegister16(rp);
                            uint8_t value = this->Read8(addr);

                            this->WriteRegister8(CPU::RegisterA, value);

                            return 7;
                        }
                    }
                } else {
                    switch (rp) {
                        case 0b11: {
                            // sta addr
                            
                            uint16_t pc = this->ReadPC();
                            this->WritePC(pc + 2);

                            uint16_t addr = this->Read16(pc);
                            uint8_t value = this->ReadRegister8(CPU::RegisterA);

                            this->Write8(addr, value);

                            return 13;
                        }

                        case 0b10: {
                            // shld addr
                            
                            uint16_t pc = this->ReadPC();
                            this->WritePC(pc + 2);

                            uint16_t addr = this->Read16(pc);
                            uint16_t value = this->ReadRegister16(CPU::RegisterPairHL);

                            this->Write16(addr, value);

                            return 16;
                        }

                        default: {
                            // stax rp
                            
                            uint16_t addr = this->ReadRegister16(rp);
                            uint8_t value = this->ReadRegister8(CPU::RegisterA);

                            this->Write8(addr, value);

                            return 7;
                        }
                    }
                }
            }

            if (opcode == 0b001) {
                if (ExtractBits8(instruction, 4, 1)) {
                    // dad rp

                    uint8_t rp = ExtractBits8(instruction, 5, 2);
                    uint16_t value = this->ReadRegister16(rp);
                    uint16_t hl = this->ReadRegister16(CPU::RegisterPairHL);
                    uint32_t sum = value + hl;

                    this->WriteRegister16(CPU::RegisterPairHL, sum & 0xFFFF);
                    this->SetFlag(CPU::Flag::C, sum > 0xFFFF);

                    return 10;
                } else {
                    // lxi rp, #imm

                    uint16_t pc = this->ReadPC();
                    this->WritePC(pc + 2);

                    uint8_t rp = ExtractBits8(instruction, 5, 2);
                    uint16_t value = this->Read16(pc);

                    this->WriteRegister16(rp, value);

                    return 10;
                }
            }

            if (ExtractBits8(instruction, 3, 1) == 0b1) {
                uint8_t opcode = ExtractBits8(instruction, 1, 2);
                uint8_t dest = ExtractBits8(instruction, 4, 3);

                if (opcode == 0b00) {
                    // inr d

                    uint8_t value = this->ReadRegister8(dest);
                    this->SetFlag(CPU::Flag::A, (value & 0xF) == 0xF);

                    value++;

                    this->WriteRegister8(dest, value);
                    this->CalculateSZP(value);

                    return dest == 0b110 ? 10 : 5;
                } else if (opcode == 0b01) {
                    // dcr d

                    uint8_t value = this->ReadRegister8(dest);
                    value--;

                    this->SetFlag(CPU::Flag::A, (value & 0xF) == 0xF);
                    this->WriteRegister8(dest, value);
                    this->CalculateSZP(value);

                    return dest == 0b110 ? 10 : 5;
                }
            } else {
                uint8_t rp = ExtractBits8(instruction, 5, 2);

                if (ExtractBits8(instruction, 4, 1) == 0b1) {
                    // dcx rp

                    uint16_t value = this->ReadRegister16(rp);
                    this->WriteRegister16(rp, value - 1);

                    return 5;
                } else {
                    // inx rp

                    uint16_t value = this->ReadRegister16(rp);
                    this->WriteRegister16(rp, value + 1);

                    return 5;
                }
            }
        } else if (field == 0b01) {
            if (instruction == 0b01110110) {
                // hlt

                this->state->SetHalt(true);

                this->Log("Halted CPU.");
                return 7;
            }

            // mov d, s

            uint8_t dest = ExtractBits8(instruction, 4, 3);
            uint8_t source = ExtractBits8(instruction, 1, 3);
            uint8_t value = this->ReadRegister8(source);

            this->WriteRegister8(dest, value);
            return (dest == 0b110 || source == 0b110) ? 7 : 5;
        } else if (field == 0b10) {
            // add s, adc s, sub s, sbc s, ana s, ora s, xra s, cmp s

            uint8_t opcode = ExtractBits8(instruction, 4, 3);
            uint8_t source = ExtractBits8(instruction, 1, 3);
            uint8_t value = this->ReadRegister8(source);

            this->Arithmetic(opcode, value);

            return source == 0b110 ? 7 : 4;
        } else {
            if (instruction == 0b11000011 || instruction == 0b11001011) {
                // jmp addr

                uint16_t pc = this->ReadPC();
                uint16_t addr = this->Read16(pc);
                this->WritePC(addr);

                return 10;
            }

            if (instruction == 0b11111011) {
                // ei

                this->state->SetInterruptsEnabled(true);

                this->Log("Enabled interrupts.");
                return 4;
            }

            if (instruction == 0b11110011) {
                // di

                this->state->SetInterruptsEnabled(false);

                this->Log("Disabled interrupts.");
                return 4;
            }

            if (instruction == 0b11100011) {
                // xthl

                uint16_t hl = this->ReadRegister16(CPU::RegisterPairHL);
                uint16_t addr = this->Pop();

                this->Push(hl);
                this->WriteRegister16(CPU::RegisterPairHL, addr);

                return 18;
            }

            if (instruction == 0b11101011) {
                // xchg

                uint16_t de = this->ReadRegister16(CPU::RegisterPairDE);
                uint16_t hl = this->ReadRegister16(CPU::RegisterPairHL);

                this->WriteRegister16(CPU::RegisterPairHL, de);
                this->WriteRegister16(CPU::RegisterPairDE, hl);

                return 5;
            }

            if (instruction == 0b11101001) {
                // pchl

                uint16_t hl = this->ReadRegister16(CPU::RegisterPairHL);
                this->WritePC(hl);

                return 5;
            }

            if (instruction == 0b11111001) {
                // sphl

                uint16_t hl = this->ReadRegister16(CPU::RegisterPairHL);
                this->WriteSP(hl);

                return 5;
            }

            if (instruction == 0b11011011) {
                // in port

                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 1);

                uint8_t port = this->Read8(pc);
                uint8_t value = this->InputData(port);

                this->WriteRegister8(CPU::RegisterA, value);
                return 10;
            }

            if (instruction == 0b11010011) {
                // out port

                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 1);

                uint8_t port = this->Read8(pc);
                uint8_t a = this->ReadRegister8(CPU::RegisterA);

                this->OutputData(port, a);
                return 10;
            }

            if (instruction == 0b11001001 || instruction == 0b11011001) {
                // ret

                this->Return();
                return 10;
            }

            uint8_t lo = instruction & 0xF;

            if (lo == 0x1) {
                // pop rp

                uint8_t rp = ExtractBits8(instruction, 5, 2);
                uint16_t value = this->Pop();

                this->WriteRegister16(rp, value, false);
                return 10;
            }

            if (lo == 0x5) {
                // push rp

                uint8_t rp = ExtractBits8(instruction, 5, 2);
                uint16_t value = this->ReadRegister16(rp, false);

                this->Push(value);
                return 11;
            }

            if (lo == 0x6) {
                // adi #imm, sui #imm, ani #imm, ori #imm

                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 1);

                uint8_t opcode = ExtractBits8(instruction, 4, 3);
                uint8_t value = this->Read8(pc);

                this->Arithmetic(opcode, value);
                return 7;
            }

            if (lo == 0xD) {
                // call addr

                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 2);

                uint16_t addr = this->Read16(pc);
                this->Call(addr);

                return 17;
            }

            if (lo == 0xE) {
                // aci #imm, sbi #imm, xri #imm, cpi #imm

                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 1);

                uint8_t opcode = ExtractBits8(instruction, 4, 3);
                uint8_t value = this->Read8(pc);

                this->Arithmetic(opcode, value);
                return 7;
            }

            uint8_t opcode = ExtractBits8(instruction, 1, 3);

            if (opcode == 0b000) {
                // rnz, rz, rnc, rc, rpo, rpe, rm, rp

                uint8_t condition = ExtractBits8(instruction, 4, 3);
                bool ret = this->ConditionMet(condition);

                if (ret)
                    this->Return();

                return ret ? 11 : 5;
            }

            if (opcode == 0b010) {
                // jnz, jz, jnc, jc, jpo, jpe, jm, jp

                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 2);

                uint8_t cond = ExtractBits8(instruction, 4, 3);

                if (this->ConditionMet(cond)) {
                    uint16_t addr = this->Read16(pc);
                    this->WritePC(addr);
                }

                return 10;
            }

            if (opcode == 0b100) {
                // cnz, cz, cnc, cc, cpo, cpe, cm, cp

                uint16_t pc = this->ReadPC();
                this->WritePC(pc + 2);

                uint8_t cond = ExtractBits8(instruction, 4, 3);

                if (this->ConditionMet(cond)) {
                    uint16_t addr = this->Read16(pc);
                    this->Call(addr);
                    return 17;
                }

                return 11;
            }

            if (opcode == 0b111) {
                // rst 0-8

                uint8_t n = ExtractBits8(instruction, 4, 3);
                this->Call(n * 8);

                return 11;
            }
        }

        throw std::runtime_error(FormatString("Unknown instruction 0x%x.", instruction));
    }

    void CPU::Push(uint16_t value)
    {
        uint16_t sp = this->ReadSP();

        uint8_t hi = value >> 8;
        uint8_t lo = value & 0xFF;

        this->Write8(sp - 1, hi);
        this->Write8(sp - 2, lo);
        this->WriteSP(sp - 2);

        this->Log("Pushed 0x%04x to stack.", value);
    }

    uint16_t CPU::Pop()
    {
        uint16_t sp = this->ReadSP();

        uint8_t hi = this->Read8(sp + 1);
        uint8_t lo = this->Read8(sp);
        this->WriteSP(sp + 2);

        uint16_t value = (hi << 8) | lo;

        this->Log("Popped 0x%04x from stack.", value);
        return value;
    }

    void CPU::Call(uint16_t addr)
    {
        uint16_t pc = this->ReadPC();
        this->Push(pc);
        this->WritePC(addr);

        this->Log("Called subroutine at addr 0x%04x (return to 0x%04x).", addr, pc);
    }

    void CPU::Return()
    {
        uint16_t addr = this->Pop();
        this->WritePC(addr);

        this->Log("Returned from subroutine to addr 0x%04x.", addr);
    }

    bool CPU::GetInteruptsEnabled() const
    {
        return this->state->GetInteruptsEnabled();
    }

    void CPU::OutputData(uint8_t port, uint8_t data) const
    {
        throw std::runtime_error("no I/O.");

        this->Log("Output 0x%x to port %x.", data, port);
    }

    uint8_t CPU::InputData(uint8_t port)
    {
        throw std::runtime_error("no I/O.");

        uint8_t data = 0;

        this->Log("Input 0x%x from port 0x%x.", data, port);
        return data;
    }

    void CPU::Arithmetic(uint8_t opcode, uint8_t value)
    {
        if (opcode == 0b000)
            this->Add(value);
        else if (opcode == 0b001)
            this->Add(value + this->GetFlag(CPU::Flag::C));
        else if (opcode == 0b010)
            this->Sub(value);
        else if (opcode == 0b011)
            this->Sub(value + this->GetFlag(CPU::Flag::C));
        else if (opcode == 0b100)
            this->And(value);
        else if (opcode == 0b101)
            this->Xor(value);
        else if (opcode == 0b110)
            this->Or(value);
        else if (opcode == 0b111)
            this->Cmp(value);
    }

    void CPU::Add(uint8_t value)
    {
        uint8_t a = this->ReadRegister8(CPU::RegisterA);
        uint16_t sum = a + value;

        this->WriteRegister8(CPU::RegisterA, sum);
        this->CalculateSZP(sum);

        this->SetFlag(CPU::Flag::C, sum > 0xFF);
        this->SetFlag(CPU::Flag::A, (a & 0xF) + (value & 0xF) > 0xF);
    }

    void CPU::Sub(uint8_t value)
    {
        uint8_t a = this->ReadRegister8(CPU::RegisterA);
        uint16_t sum = a - value;

        this->WriteRegister8(CPU::RegisterA, sum);
        this->CalculateSZP(sum);

        this->SetFlag(CPU::Flag::C, value > a);
        this->SetFlag(CPU::Flag::A, (a & 0xF) > (value & 0xF));
    }

    void CPU::And(uint8_t value)
    {
        uint8_t a = this->ReadRegister8(CPU::RegisterA);
        uint8_t n = a & value;

        this->WriteRegister8(CPU::RegisterA, n);
        this->CalculateSZP(n);

        this->SetFlag(CPU::Flag::C, 0);
        // this->SetFlag(CPU::Flag::A, 0);
    }

    void CPU::Or(uint8_t value)
    {
        uint8_t a = this->ReadRegister8(CPU::RegisterA);
        uint8_t n = a | value;

        this->WriteRegister8(CPU::RegisterA, n);
        this->CalculateSZP(n);

        this->SetFlag(CPU::Flag::C, 0);
        this->SetFlag(CPU::Flag::A, 0);
    }

    void CPU::Xor(uint8_t value)
    {
        uint8_t a = this->ReadRegister8(CPU::RegisterA);
        uint8_t n = a ^ value;

        this->WriteRegister8(CPU::RegisterA, n);
        this->CalculateSZP(n);

        this->SetFlag(CPU::Flag::C, 0);
        this->SetFlag(CPU::Flag::A, 0);
    }

    void CPU::Cmp(uint8_t value)
    {
        uint8_t a = this->ReadRegister8(CPU::RegisterA);
        uint16_t sum = a - value;
        this->CalculateSZP(sum);

        this->SetFlag(CPU::Flag::C, value > a);
        this->SetFlag(CPU::Flag::A, (a & 0xF) > (value & 0xF));
    }
}