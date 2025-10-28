#ifndef CPU_TRIT_H
#define CPU_TRIT_H

#include "ternary.h"

#define REGISTER_SIZE 16
#define INST_ROM_SIZE 512
#define DATA_RAM_SIZE 512

struct cpu_trit {
    struct trit16 reg[REGISTER_SIZE];
    struct trit16 pc;
    struct trit8 inst_rom[INST_ROM_SIZE];
    struct trit8 data_ram[DATA_RAM_SIZE];
    trit flag_sign;
    trit flag_overflow;
    trit flag_zero;
    trit flag_carry;
};

#endif  /* CPU_TRIT_H */
