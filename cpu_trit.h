#ifndef CPU_TRIT_H
#define CPU_TRIT_H

#include "ternary.h"

#define REGISTER_SIZE 16 	 /* количество троичных регистров процессора */ 

#define INST_ROM_SIZE 243  /* для троичного числа адреса ROM */
#define DATA_RAM_SIZE 243  /* для троичного числа адреса RAM */

struct cpu_trit {
    /* Модель процессора TRIT RISC-V */
    tr32 pc;
    tr32 reg[REGISTER_SIZE];    
    tr8  inst_rom_arr[INST_ROM_SIZE];
    tr8  data_ram_arr[DATA_RAM_SIZE];
    /* Указатели на память с возвожностью отрицательных индексов */
    tr8 * inst_rom;
    tr8 * data_ram;
    /* Приватные триты при выполнении команд */
    trit flag_sign;			
    trit flag_overflow;
    trit flag_zero;
    trit flag_carry;
};

#endif  /* CPU_TRIT_H */
