#ifndef INST_TRIT_H
#define INST_TRIT_H

#include <stdint.h>

#include "ternary.h"
#include "inst.h"

extern const struct inst_data inst_list[];


/* Троичные */
void trs_pc_update(struct cpu_trit *c, tr16 offset);
void trs_pc_write(struct cpu_trit *c, tr16 addr);
tr16 trs_pc_read(struct cpu_trit *c);

//viv+ dbg
void trs_reg_write(struct cpu_trit *c, uint8_t reg_idx, tr16 data);
void trs_reg_read(struct cpu_trit *c, uint8_t reg_idx, tr16 * tr );

void trs_rom_read_w(struct cpu_trit *c, tr16 *tr);

void trs_mem_read_b(struct cpu_trit *c, tr16 addr, tr8 *tr);
void trs_mem_read_w(struct cpu_trit *c, tr16 addr, tr16 *tr);


/* Отладка команд троичного процессра */
void trs_dbg_oper(struct cpu_trit *c);

#endif /* INST_TRIT_H */
