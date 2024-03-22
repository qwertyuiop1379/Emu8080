#include "Encode.h"

#include "Util.h"
#include "CPU.h"

namespace Emu8080 {
    uint8_t Encode::IsValidConditionCode(const std::string &_str)
    {
        std::string str = LowercaseString(_str);
        return str == "nz" || str == "z" || str == "nc" || str == "c" || str == "po" || str == "pe" || str == "p" || str == "m";
    }

    uint8_t Encode::ReadConditionCode(const std::string &_str)
    {
        auto str = LowercaseString(_str);

        if (str == "nz")
            return 0;
        else if (str == "z")
            return 1;
        else if (str == "nc")
            return 2;
        else if (str == "c")
            return 3;
        else if (str == "po")
            return 4;
        else if (str == "pe")
            return 5;
        else if (str == "p")
            return 6;
        else if (str == "m")
            return 7;
        else
            throw std::runtime_error(FormatString("Invalid condition code '%s'.", _str.c_str()));
    }

    Encode::ArgumentFormat Encode::DetermineFormat(const std::string &instr)
    {
        if (instr == "mov")
            return Encode::ArgumentFormat::TwoRegister8s;
        else if (instr == "mvi")
            return Encode::ArgumentFormat::OneRegister8OneImmediate8;
        else if (instr == "lxi")
            return Encode::ArgumentFormat::OneRegister16OneImmediate16;
        else if (instr == "lda" || instr == "sta")
            return Encode::ArgumentFormat::OneImmediate16;
        else if (instr == "lhld" || instr == "shld")
            return Encode::ArgumentFormat::OneImmediate16;
        else if (instr == "ldax" || instr == "stax")
            return Encode::ArgumentFormat::OneRegister16Restricted;
        else if (instr == "xchg")
            return Encode::ArgumentFormat::InstructionOnly;
        else if (instr == "add" || instr == "adc")
            return Encode::ArgumentFormat::OneRegister8;
        else if (instr == "adi" || instr == "aci")
            return Encode::ArgumentFormat::OneImmediate8;
        else if (instr == "sub" || instr == "sbb")
            return Encode::ArgumentFormat::OneRegister8;
        else if (instr == "sui" || instr == "sbi")
            return Encode::ArgumentFormat::OneImmediate8;
        else if (instr == "inr" || instr == "dcr")
            return Encode::ArgumentFormat::OneRegister8;
        else if (instr == "inx" || instr == "dcx")
            return Encode::ArgumentFormat::OneRegister16;
        else if (instr == "dad")
            return Encode::ArgumentFormat::OneRegister16;
        else if (instr == "daa")
            return Encode::ArgumentFormat::InstructionOnly;
        else if (instr == "ana" || instr == "ora" || instr == "xra" || instr == "cmp")
            return Encode::ArgumentFormat::OneRegister8;
        else if (instr == "ani" || instr == "ori" || instr == "xri" || instr == "cpi")
            return Encode::ArgumentFormat::OneImmediate8;
        else if (instr == "rlc" || instr == "rrc" || instr == "ral" || instr == "rar" || instr == "cma" || instr == "cmc" || instr == "stc")
            return Encode::ArgumentFormat::InstructionOnly;
        else if (instr == "jmp" || instr == "call")
            return Encode::ArgumentFormat::OneImmediate16;
        else if (instr == "ret")
            return Encode::ArgumentFormat::InstructionOnly;
        else if ((*instr.c_str() == 'j' || *instr.c_str() == 'c') && Encode::IsValidConditionCode(instr.substr(1)))
            return Encode::ArgumentFormat::OneConditionCodeOneImmediate16;
        else if (*instr.c_str() == 'r' && Encode::IsValidConditionCode(instr.substr(1)))
            return Encode::ArgumentFormat::OneConditionCode;
        else if (instr == "rst")
            return Encode::ArgumentFormat::OneVector;
        else if (instr == "pchl")
            return Encode::ArgumentFormat::InstructionOnly;
        else if (instr == "push")
            return Encode::ArgumentFormat::OneRegister16PSWAllowed;
        else if (instr == "pop")
            return Encode::ArgumentFormat::OneRegister16PSWAllowed;
        else if (instr == "xthl" || instr == "sphl")
            return Encode::ArgumentFormat::InstructionOnly;
        else if (instr == "in" || instr == "out")
            return Encode::ArgumentFormat::OnePort;
        else if (instr == "ei" || instr == "di")
            return Encode::ArgumentFormat::InstructionOnly;
        else if (instr == "hlt" || instr == "nop")
            return Encode::ArgumentFormat::InstructionOnly;
        else
            throw std::runtime_error(FormatString("Invalid instruction '%s'.", instr.c_str()));
    }

    bool Encode::ExtractArguments(const std::string &_str, Encode::ArgumentFormat format, uint16_t *arg1, uint16_t *arg2)
    {
        auto _s = _str.c_str();
        while (*_s && *_s != ' ') {
            if (!isalpha(*_s))
                return false;
            _s++;
        }
        
        auto str = RemoveWhitespaceFromString(LowercaseString(_s));

        switch (format) {
            case Encode::ArgumentFormat::InstructionOnly: {
                auto s = str.c_str();

                while (*s) {
                    if (!isalpha(*s))
                        return false;
                    s++;
                }

                return true;
            }

            case Encode::ArgumentFormat::OneRegister8: {
                char *s = (char *)str.c_str();

                if (*(s + 2) != '\0')
                    return false;

                char c = *s;
                    
                if ((!(c >= 'a' && c <= 'e') && c != 'h' && c != 'l' && c != 'm'))
                    return false;

                *arg1 = Register8ForString(std::string(&c, 1));
                return true;
            }

            case Encode::ArgumentFormat::TwoRegister8s: {
                char *s = (char *)str.c_str();

                char c = *s;
                    
                if ((!(c >= 'a' && c <= 'e') && c != 'h' && c != 'l' && c != 'm'))
                    return false;

                if (*(++s) != ',')
                    return false;

                uint8_t reg1 = Register8ForString(std::string(&c, 1));
                c = *(++s);
                    
                if ((!(c >= 'a' && c <= 'e') && c != 'h' && c != 'l' && c != 'm'))
                    return false;

                if (*(++s) != '\0')
                    return false;

                *arg1 = reg1;
                *arg2 = Register8ForString(std::string(&c, 1));

                return true;
            }

            case Encode::ArgumentFormat::OneRegister16PSWAllowed:
            case Encode::ArgumentFormat::OneRegister16Restricted:
            case Encode::ArgumentFormat::OneRegister16: {
                char *s = (char *)str.c_str();
                char c = *s;
                    
                if (c == 's') {
                    if (format == Encode::ArgumentFormat::OneRegister16PSWAllowed)
                        return false;

                    if (*(++s) != 'p' && *(++s) != '\0')
                        return false;

                    *arg1 = CPU::RegisterPairSP;
                    return true;
                }

                if (c == 'p') {
                    if (format != Encode::ArgumentFormat::OneRegister16PSWAllowed)
                        return false;

                    if (*(++s) == 's' && *(++s) == 'w' && *(++s) == '\0') {
                        *arg1 = CPU::RegisterPairPSW;
                        return true;
                    } else return false;
                }

                if (*(s + 2) != '\0')
                    return false;

                if (c != 'b' && c != 'd' && c != 'h')
                    return false;

                if (format == Encode::ArgumentFormat::OneRegister16Restricted) {
                    if (c != 'b' && c != 'd')
                        return false;
                }

                *arg1 = Register16ForString(std::string(&c, 1));
                return true;
            }

            case Encode::ArgumentFormat::OneImmediate8Restricted:
            case Encode::ArgumentFormat::OneImmediate8: {
                char *s = (char *)str.c_str();
                
                if (*s != '$' || strlen(++s) > 2 || !IsValidHexString(s))
                    return false;

                uint8_t n = (uint8_t)UIntFromHex(s);

                if (format == Encode::ArgumentFormat::OneImmediate8Restricted && n > 0b111)
                    return false;

                *arg1 = n;
                return true;
            }
            
            case Encode::ArgumentFormat::OneImmediate16: {
                char *s = (char *)str.c_str();
                
                if (*s != '$' || strlen(++s) > 4 || !IsValidHexString(s))
                    return false;

                *arg1 = (uint16_t)UIntFromHex(s);
                return true;
            }

            case Encode::ArgumentFormat::OneVector:
            case Encode::ArgumentFormat::OnePort: {
                char *s = (char *)str.c_str();
                
                if (strlen(s) > (format == Encode::ArgumentFormat::OnePort ? 2 : 1) || !IsValidHexString(s))
                    return false;

                *arg1 = (uint16_t)UIntFromHex(s);
                return true;
            }

            case Encode::ArgumentFormat::OneRegister8OneImmediate8: {
                char *s = (char *)str.c_str();
                char c = *s;
                    
                if (!(c >= 'a' && c <= 'e') && c != 'h' && c != 'l' && c != 'm')
                    return false;

                if (*(++s) != ',')
                    return false;
                
                if (*(++s) != '$' || strlen(++s) > 2 || !IsValidHexString(s))
                    return false;

                *arg1 = Register8ForString(std::string(&c, 1));
                *arg2 = (uint8_t)UIntFromHex(s);

                return true;
            }

            case Encode::ArgumentFormat::OneRegister16OneImmediate16: {
                char *s = (char *)str.c_str();
                char c = *(s++);

                uint8_t reg;
                    
                if (c == 's') {
                    if (*(s++) != 'p')
                        return false;

                    reg = CPU::RegisterPairSP;
                } else {
                    if (c != 'b' && c != 'd' && c != 'h')
                        return false;

                    reg = Register16ForString(std::string(&c, 1));
                }
                
                if (*(s++) != ',' || *(s++) != '$' || strlen(s) > 4 || !IsValidHexString(s))
                    return false;

                *arg1 = reg;
                *arg2 = (uint16_t)UIntFromHex(s);

                return true;
            }

            case Encode::ArgumentFormat::OneConditionCode: {
                std::string code;
                char *s = (char *)_str.c_str() + 1;

                while (*s && *s != ' ')
                    code += *(s++);

                *arg1 = Encode::ReadConditionCode(code);
                return true;
            }

            case Encode::ArgumentFormat::OneConditionCodeOneImmediate16: {
                std::string code;
                char *s = (char *)_str.c_str() + 1;

                while (*s && *s != ' ')
                    code += *(s++);

                uint8_t condition = Encode::ReadConditionCode(code);
                
                s = (char *)str.c_str();
                
                if (*s != '$' || strlen(++s) > 4 || !IsValidHexString(s))
                    return false;

                *arg1 = condition;
                *arg2 = (uint16_t)UIntFromHex(s);

                return true;
            }

            default: return false;
        }
    }

    static uint8_t BuildInstruction(uint8_t *bytes, const std::string &instr, Encode::ArgumentFormat format, uint16_t arg1, uint16_t arg2)
    {
        if (instr == "mov") {
            bytes[0] = 0b01000000 | (arg1 << 3) | arg2;
            return 1;
        } else if (instr == "mvi") {
            bytes[0] = 0b110 | (arg1 << 3);
            bytes[1] = arg2;
            return 2;
        } else if (instr == "lxi") {
            bytes[0] = 0x01 | (arg1 << 4);
            bytes[1] = arg2 & 0xFF;
            bytes[2] = arg2 >> 8;
            return 3;
        } else if (instr == "lda") {
            bytes[0] = 0b00111010;
            bytes[1] = arg1 & 0xFF;
            bytes[2] = arg1 >> 8;
            return 3;
        } else if (instr == "sta") {
            bytes[0] = 0b00110010;
            bytes[1] = arg1 & 0xFF;
            bytes[2] = arg1 >> 8;
            return 3;
        } else if (instr == "lhld") {
            bytes[0] = 0b00101010;
            bytes[1] = arg1 & 0xFF;
            bytes[2] = arg1 >> 8;
            return 3;
        } else if (instr == "shld") {
            bytes[0] = 0b00100010;
            bytes[1] = arg1 & 0xFF;
            bytes[2] = arg1 >> 8;
            return 3;
        } else if (instr == "ldax") {
            bytes[0] = 0b1010 | (arg1 << 4);
            return 1;
        } else if (instr == "stax") {
            bytes[0] = 0b10 | (arg1 << 4);
            return 1;
        } else if (instr == "xchg") {
            bytes[0] = 0b11101011;
            return 1;
        } else if (instr == "add") {
            bytes[0] = 0b10000000 | arg1;
            return 1;
        } else if (instr == "adi") {
            bytes[0] = 0b11000110;
            bytes[1] = arg1 & 0xFF;
            return 2;
        } else if (instr == "adc") {
            bytes[0] = 0b10001000 | arg1;
            return 1;
        } else if (instr == "aci") {
            bytes[0] = 0b11001110;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "sub") {
            bytes[0] = 0b10010000 | arg1;
            return 1;
        } else if (instr == "sui") {
            bytes[0] = 0b11010110;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "sbb") {
            bytes[0] = 0b10011000 | arg1;
            return 1;
        } else if (instr == "sbi") {
            bytes[0] = 0b11011110;
            bytes[1] = arg1;
            return 1;
        } else if (instr == "inr") {
            bytes[0] = 0b100 | (arg1 << 3);
            return 1;
        } else if (instr == "dcr") {
            bytes[0] = 0b101 | (arg1 << 3);
            return 1;
        } else if (instr == "inx") {
            bytes[0] = 0b11 | (arg1 << 4);
            return 1;
        } else if (instr == "dcx") {
            bytes[0] = 0b1011 | (arg1 << 4);
            return 1;
        } else if (instr == "dad") {
            bytes[0] = 0b00001001 | (arg1 << 4);
            return 1;
        } else if (instr == "daa") {
            bytes[0] = 0b00100111;
            return 1;
        } else if (instr == "ana") {
            bytes[0] = 0b10100000 | arg1;
            return 1;
        } else if (instr == "ora") {
            bytes[0] = 0b10110000 | arg1;
            return 1;
        } else if (instr == "xra") {
            bytes[0] = 0b10101000 | arg1;
            return 1;
        } else if (instr == "cmp") {
            bytes[0] = 0b10111000 | arg1;
            return 1;
        } else if (instr == "ani") {
            bytes[0] = 0b11100110;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "ori") {
            bytes[0] = 0b11110110;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "xri") {
            bytes[0] = 0b11101110;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "cpi") {
            bytes[0] = 0b11111110;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "rlc") {
            bytes[0] = 0b00000111;
            return 1;
        } else if (instr == "rrc") {
            bytes[0] = 0b00001111;
            return 1;
        } else if (instr == "ral") {
            bytes[0] = 0b00010111;
            return 1;
        } else if (instr == "rar") {
            bytes[0] = 0b00011111;
            return 1;
        } else if (instr == "cma") {
            bytes[0] = 0b00101111;
            return 1;
        } else if (instr == "cmc") {
            bytes[0] = 0b00111111;
            return 1;
        } else if (instr == "stc") {
            bytes[0] = 0b00110111;
            return 1;
        } else if (instr == "jmp") {
            bytes[0] = 0b11000011;
            bytes[1] = arg1 & 0xFF;
            bytes[2] = arg1 >> 8;
            return 3;
        } else if (instr == "call") {
            bytes[0] = 0b11001101;
            bytes[1] = arg1 & 0xFF;
            bytes[2] = arg1 >> 8;
            return 3;
        } else if (instr == "ret") {
            bytes[0] = 0b11001001;
            return 1;
        } else if (*instr.c_str() == 'j' && Encode::IsValidConditionCode(instr.substr(1))) {
            bytes[0] = 0b11000010 | (arg1 << 3);
            bytes[1] = arg2 & 0xFF;
            bytes[2] = arg2 >> 8;
            return 3;
        } else if (*instr.c_str() == 'c' && Encode::IsValidConditionCode(instr.substr(1))) {
            bytes[0] = 0b11000100 | (arg1 << 3);
            bytes[1] = arg2 & 0xFF;
            bytes[2] = arg2 >> 8;
            return 3;
        } else if (*instr.c_str() == 'r' && Encode::IsValidConditionCode(instr.substr(1))) {
            bytes[0] = 0b11000000 | (arg1 << 3);
            return 1;
        } else if (instr == "rst") {
            bytes[0] = 0b11000111 | (arg1 << 3);
            return 1;
        } else if (instr == "pchl") {
            bytes[0] = 0b11101001;
            return 1;
        } else if (instr == "push") {
            bytes[0] = 0b11000101 | (arg1 << 4);
            return 1;
        } else if (instr == "pop") {
            bytes[0] = 0b11000001 | (arg1 << 4);
            return 1;
        } else if (instr == "xthl") {
            bytes[0] = 0b11100011;
            return 1;
        } else if (instr == "sphl") {
            bytes[0] = 0b11111001;
            return 1;
        } else if (instr == "in") {
            bytes[0] = 0b11011011;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "out") {
            bytes[0] = 0b11010011;
            bytes[1] = arg1;
            return 2;
        } else if (instr == "ei") {
            bytes[0] = 0b11111011;
            return 1;
        } else if (instr == "di") {
            bytes[0] = 0b11110011;
            return 1;
        } else if (instr == "hlt") {
            bytes[0] = 0b01110110;
            return 1;
        } else if (instr == "nop") {
            bytes[0] = 0b00000000;
            return 1;
        }

        throw std::runtime_error(FormatString("Invalid instruction '%s'.", instr.c_str()));
    }

    void Encode::EncodeInstruction(const std::string &_str, uint8_t * const buffer, uint8_t *size)
    {
        std::string str = LowercaseString(_str);
        str = TrimSurroundingWhitespace(str);

        uint8_t bytes[3] = { 0x00, 0x00, 0x00 };
        uint8_t count = 1;

        std::string instr;
        auto s = str.c_str();

        while (*s == ' ')
            s++;
        while (*s && *s != ' ')
            instr += *(s++);

        auto format = DetermineFormat(instr);

        uint16_t arg1 = 0;
        uint16_t arg2 = 0;

        if (Encode::ExtractArguments(str, format, &arg1, &arg2) == false)
            throw std::runtime_error(FormatString("Failed to read arguments from string '%s'.", str.c_str()));

        BuildInstruction(bytes, instr, format, arg1, arg2);

        buffer[0] = bytes[0];
        buffer[1] = bytes[1];
        buffer[2] = bytes[2];
        
        if (size != nullptr)
            *size = count;
    }

    std::string Encode::DecodeInstruction(const uint8_t * const bytes, uint8_t *_count)
    {
        static const char *instructions[] =
        {
            "nop", "lxi b,", "stax b", "inx b",
            "inr b", "dcr b", "mvi b,", "rlc", "nop", "dad b", "ldax b", "dcx b",
            "inr c", "dcr c", "mvi c,", "rrc", "nop", "lxi d,", "stax d", "inx d",
            "inr d", "dcr d", "mvi d,", "ral", "nop", "dad d", "ldax d", "dcx d",
            "inr e", "dcr e", "mvi e,", "rar", "nop", "lxi h,", "shld", "inx h",
            "inr h", "dcr h", "mvi h,", "daa", "nop", "dad h", "lhld", "dcx h",
            "inr l", "dcr l", "mvi l,", "cma", "nop", "lxi sp,", "sta", "inx sp",
            "inr M", "dcr M", "mvi M,", "stc", "nop", "dad sp", "lda", "dcx sp",
            "inr a", "dcr a", "mvi a,", "cmc", "mov b, b", "mov b, c", "mov b, d",
            "mov b, e", "mov b, h", "mov b, l", "mov b, M", "mov b, a", "mov c, b", "mov c, c",
            "mov c, d", "mov c, e", "mov c, h", "mov c, l", "mov c, M", "mov c, a", "mov d, b",
            "mov d, c", "mov d, d", "mov d, e", "mov d, h", "mov d, l", "mov d, M", "mov d, a",
            "mov e, b", "mov e, c", "mov e, d", "mov e, e", "mov e, h", "mov e, l", "mov e, M",
            "mov e, a", "mov h, b", "mov h, c", "mov h, d", "mov h, e", "mov h, h", "mov h, l",
            "mov h, M", "mov h, a", "mov l, b", "mov l, c", "mov l, d", "mov l, e", "mov l, h",
            "mov l, l", "mov l, M", "mov l, a", "mov M, b", "mov M, c", "mov M, d", "mov M, e",
            "mov M, h", "mov M, l", "hlt", "mov M, a", "mov a, b", "mov a, c", "mov a, d",
            "mov a, e", "mov a, h", "mov a, l", "mov a, M", "mov a, a", "add b", "add c",
            "add d", "add e", "add h", "add l", "add M", "add a", "adc b", "adc c",
            "adc d", "adc e", "adc h", "adc l", "adc M", "adc a", "sub b", "sub c",
            "sub d", "sub e", "sub h", "sub l", "sub M", "sub a", "sbb b", "sbb c",
            "sbb d", "sbb e", "sbb h", "sbb l", "sbb M", "sbb a", "ana b", "ana c",
            "ana d", "ana e", "ana h", "ana l", "ana M", "ana a", "xra b", "xra c",
            "xra d", "xra e", "xra h", "xra l", "xra M", "xra a", "ora b", "ora c",
            "ora d", "ora e", "ora h", "ora l", "ora M", "ora a", "cmp b", "cmp c",
            "cmp d", "cmp e", "cmp h", "cmp l", "cmp M", "cmp a", "rnz", "pop b",
            "jnz", "jmp", "cnz", "push b", "adi", "rst 0", "rz", "ret", "jz",
            "jmp", "cz", "call", "aci", "rst 1", "rnc", "pop d", "jnc", "out",
            "cnc", "push d", "sui", "rst 2", "rc", "ret", "jc", "in", "cc",
            "call", "sbi", "rst 3", "rpo", "pop h", "jpo", "xthl", "cpo", "push h",
            "ani", "rst 4", "rpe", "pchl", "jpe", "xchg", "cpe", "call", "xri",
            "rst 5", "rp", "pop psw", "jp", "di", "cp", "push psw", "ori",
            "rst 6", "rm", "sphl", "jm", "ei", "cm", "call", "cpi", "rst 7"
        };

        size_t count = 1;
        uint8_t instruction = bytes[0];

        int32_t immediate = -1;

        if (instruction == 0xD3 || instruction == 0xDB || ((instruction & 0b111) == 0b110 && ((instruction & 0xF0) < 0x40 || (instruction & 0xF0) > 0xB0))) {
            count = 2;
            immediate = bytes[1];
        }

        if (((instruction & 0xCF) == 0x01) || instruction == 0x22 || instruction == 0x32 || instruction == 0x2A || instruction == 0x3A || instruction == 0xC3 || instruction == 0xCB
         || ((instruction & 0xCF) == 0xC2) || ((instruction & 0xCF) == 0xC4) || ((instruction & 0xCF) == 0xCA) || ((instruction & 0xCF) == 0xCC) || ((instruction & 0xCF) == 0xCD)) {
            count = 3;
            immediate = (bytes[1]) | (bytes[2] << 8);
        }

        if (_count != nullptr)
            *_count = count;

        std::string decode = instructions[instruction];

        if (immediate != -1) {
            if (instruction == 0xD3 || instruction == 0xDB || (instruction & 0xC7) == 0xC7)
                decode += FormatString(" %x", immediate);
            else
                decode += FormatString(" $%x", immediate);
        }

        return decode;
    }
}