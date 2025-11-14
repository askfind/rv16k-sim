#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "cpu_trit.h"
#include "bitpat.h"
#include "inst.h"
#include "inst_trit.h"
#include "log.h"

extern int flag_inst_trits;

/* -------------------------------------------------
*  Реализация инструкций для двоичного режима RISC-V
*/

void pc_update(struct cpu_trit *c, uint16_t offset){
    uint16_t pc = 0;

    pc =  trits_to_uint16( &c->pc );    
    pc = pc + offset;
    
	uint16_to_trits(&c->pc, pc);	
		
    log_printf("PC => 0x%04X ", pc);
}


void pc_write(struct cpu_trit *c, uint16_t addr){
    
    uint16_t pc = 0;
    
    uint16_to_trits(&c->pc, pc);    
    
    log_printf("PC => 0x%08X ", pc);
}

uint16_t pc_read(struct cpu_trit *c){
    uint16_t pc = 0;
    
    pc =  trits_to_uint16(&c->pc);
        
    return pc;
}

void reg_write(struct cpu_trit *c, uint8_t reg_idx, uint16_t data){
    uint16_to_trits( &c->reg[reg_idx], data);        
    log_printf("Reg x%d => 0x%04X ", reg_idx, data);
}

uint16_t reg_read(struct cpu_trit *c, uint8_t reg_idx){
    uint16_t r;
    r  = trits_to_uint16(&c->reg[reg_idx]);
    return r;
}

void mem_write_b(struct cpu_trit *c, uint16_t addr, uint8_t data){
    log_printf("DataRam[0x%04X] => 0x%04X ", addr, data);

    assert(addr < DATA_RAM_SIZE && "RAM write to invalid address!");

    uint8_to_trits(&c->data_ram[addr], data);
}

void mem_write_w(struct cpu_trit *c, uint16_t addr, uint16_t data){
    log_printf("DataRam[0x%04X] => 0x%04X ", addr, data&0xFF);
    log_printf("DataRam[0x%04X] => 0x%04X ", addr+1, data>>8);

    assert(addr < DATA_RAM_SIZE && "RAM write to invalid address!");
    
    uint8_to_trits(&c->data_ram[addr], data&0xFF);
    uint8_to_trits(&c->data_ram[addr+1], data>>8);
}

uint8_t mem_read_b(struct cpu_trit *c, uint16_t addr){
    assert(addr < DATA_RAM_SIZE - 1 && "RAM read from invalid address!");
    return trits_to_uint8(&c->data_ram[addr]);
}

uint16_t mem_read_w(struct cpu_trit *c, uint16_t addr){
    assert(addr < DATA_RAM_SIZE - 1 && "RAM read from invalid address!");
    return trits_to_uint8(&c->data_ram[addr]) + (trits_to_uint8(&c->data_ram[addr+1])<<8);
}

uint16_t rom_read_w(struct cpu_trit *c){
    uint16_t r;
    uint16_t pc;
    
    pc = trits_to_uint16(&c->pc);
    
    assert(pc < INST_ROM_SIZE - 1 && "ROM read from invalid address!");    
    r = trits_to_uint8(&c->inst_rom[pc]) + (trits_to_uint8(&c->inst_rom[pc+1])<<8);
    return r;
}

/*
 * Выделить биты [s...e]
 */ 
uint16_t get_bits(uint16_t t, int s, int e){
    int bit_len = e-s;
    uint32_t bit_mask = 1;
    for(int i=0;i<bit_len;i++){
        bit_mask = (bit_mask<<1)+1;
    }
    return (t>>s)&bit_mask;
}

uint16_t sign_ext(uint16_t t, uint8_t sign_bit){
    uint16_t sign_v = 0;
    uint16_t sign = get_bits(t, sign_bit, sign_bit);
    for(int i=sign_bit;i<16;i++){
        sign_v |= (sign<<i);
    }
    return t|sign_v;
}

uint8_t flag_zero(uint16_t res){
    return res == 0;
}

uint8_t flag_sign(uint16_t res){
    return get_bits(res, 15, 15);
}

uint8_t flag_overflow(uint16_t s1, uint16_t s2, uint16_t res){
    uint8_t s1_sign = get_bits(s1, 15, 15);
    uint8_t s2_sign = get_bits(s2, 15, 15);
    uint8_t res_sign = get_bits(res, 15, 15);
    return ((s1_sign^s2_sign) == 0)&((s2_sign^res_sign) == 1);
}

void inst_lw(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LW\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);
    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    uint16_t s_data = reg_read(c, rs);
    uint16_t res = imm+s_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(imm, s_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    reg_write(c, rd, mem_read_w(c, res&0xFFFF));
    pc_update(c, 2);
}

void inst_lwsp(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LWSP\t");

    //viv- no used  uint8_t rs = (uint8_t)(get_bits(inst, 4, 7));
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t imm = (get_bits(inst, 4, 11)<<1);
    uint16_t d_data = reg_read(c, 1);
    uint16_t res = imm+d_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(imm, d_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    reg_write(c, rd, mem_read_w(c, res&0xFFFF));
    pc_update(c, 2);
}

void inst_lbu(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LBU\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);
    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    uint16_t s_data = reg_read(c, rs);
    uint16_t res = imm+s_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = flag_sign(res&0xFFFF);
    c->flag_overflow = flag_overflow(imm, s_data, res&0xFFFF);
    c->flag_zero = flag_zero(res&0xFFFF);
    reg_write(c, rd, mem_read_b(c, res&0xFFFF));
    pc_update(c, 2);
}

void inst_lb(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LB\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);
    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    uint16_t s_data = reg_read(c, rs);
    uint16_t res = imm+s_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = flag_sign(res&0xFFFF);
    c->flag_overflow = flag_overflow(imm, s_data, res&0xFFFF);
    c->flag_zero = flag_zero(res&0xFFFF);
    reg_write(c, rd, sign_ext(mem_read_b(c, res&0xFFFF), 7));
    pc_update(c, 2);
}

void inst_lh(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LH\t"); 
} 

void inst_lhu(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LHU\t"); 
} 

void inst_slti(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SLTI\t"); 
} 

void inst_sltiu(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SLTIU\t"); 
} 

void inst_xori(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: XORI\t"); 
} 

void inst_srli(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SRLI\t"); 
} 

void inst_srai(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SRAI\t"); 
} 

void inst_ori(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SORI\t"); 
} 

void inst_andi(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SANDI\t"); 
} 

void inst_auipc(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SAUIPC\t"); 
} 

void inst_sh(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SH\t"); 
} 

void inst_sll(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SLL\t"); 
} 

void inst_slt(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SLT\t"); 
} 

void inst_sltu(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SLTU\t"); 
} 

void inst_srl(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SRL\t"); 
} 

void inst_sra(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SRA\t"); 
} 

void inst_lui(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LUI\t"); 
} 

void inst_beq(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: BEQ\t"); 
} 

void inst_bne(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: BNE\t"); 
} 

void inst_blt(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: BLT\t"); 
} 

void inst_bge(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: BGE\t"); 
} 

void inst_bltu(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: BLTU\t"); 
} 

void inst_bgeu(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: BGEU\t"); 
} 

void inst_slli(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SLLI\t"); 
} 


 
void inst_sw(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SW\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);
    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res = imm+d_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(imm, d_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    mem_write_w(c, res&0xFFFF, reg_read(c, rs));
    pc_update(c, 2);
}

void inst_swsp(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SWSP\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t imm = (get_bits(inst, 8, 11)<<5)+(rd<<1);
    uint16_t s_data = reg_read(c, 1);
    uint16_t res = s_data+imm;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(imm, s_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    mem_write_w(c, res&0xFFFF, reg_read(c, rs));
    pc_update(c, 2);
}

void inst_sb(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SB\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);
    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res = imm+d_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(imm, d_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    mem_write_b(c, res&0xFFFF, reg_read(c, rs)&0xFF);
    pc_update(c, 2);
}

void inst_mov(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: MOV\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    reg_write(c, rd, s_data);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(s_data&0xFFFF));
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(flag_zero(s_data&0xFFFF));
    pc_update(c, 2);
}

void inst_add(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: ADD\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    uint16_t d_data = reg_read(c, rd);
    uint32_t res = s_data+d_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    reg_write(c, rd, res&0xFFFF);
    pc_update(c, 2);
}

void inst_sub(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: SUB\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = (~reg_read(c, rs))+1;
    uint16_t d_data = reg_read(c, rd);
    uint32_t res = s_data+d_data;
    if(res > 0xFFFF || s_data == 0){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    reg_write(c, rd, res&0xFFFF);
    pc_update(c, 2);
}

void inst_and(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: AND\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res_w = s_data&d_data;
    reg_write(c, rd, res_w);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(res_w));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res_w));
    c->flag_zero = getbool(flag_zero(res_w));
    pc_update(c, 2);
}

void inst_or(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: OR\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res_w = s_data|d_data;
    reg_write(c, rd, res_w);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(res_w));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res_w));
    c->flag_zero = getbool(flag_zero(res_w));
    pc_update(c, 2);
}

void inst_xor(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: XOR\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res_w = s_data^d_data;
    reg_write(c, rd, res_w);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(res_w));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res_w));
    c->flag_zero = getbool(flag_zero(res_w));
    pc_update(c, 2);
}

void inst_lsl(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LSL\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res_w = d_data << s_data;
    reg_write(c, rd, res_w);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(res_w));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res_w));
    c->flag_zero = getbool(flag_zero(res_w));
    pc_update(c, 2);
}

void inst_lsr(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LSR\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res_w = d_data >> s_data;
    reg_write(c, rd, res_w);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(res_w));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res_w));
    c->flag_zero = getbool(flag_zero(res_w));
    pc_update(c, 2);
}

void inst_asr(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: ASR\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = reg_read(c, rs);
    uint16_t d_data = reg_read(c, rd);
    uint16_t res_w = ((int16_t)d_data) >> s_data;
    reg_write(c, rd, res_w);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(res_w));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res_w));
    c->flag_zero = getbool(flag_zero(res_w));
    pc_update(c, 2);
}

void inst_cmp(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: CMP\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = (~reg_read(c, rs))+1;
    uint16_t d_data = reg_read(c, rd);
    uint32_t res = s_data+d_data;
    if(res > 0xFFFF || s_data == 0){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = flag_sign(res&0xFFFF);
    c->flag_overflow = flag_overflow(s_data, d_data, res&0xFFFF);
    c->flag_zero = flag_zero(res&0xFFFF);
    pc_update(c, 2);
}

void inst_li(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: LI\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);
    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(flag_sign(imm&0xFFFF));
    c->flag_overflow = 0;
    c->flag_zero = getbool(flag_zero(imm&0xFFFF));
    reg_write(c, rd, imm);
    pc_update(c, 2);
}

void inst_addi(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: ADDI\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = sign_ext(rs, 3);
    uint16_t d_data = reg_read(c, rd);
    uint32_t res = s_data+d_data;
    if(res > 0xFFFF){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    reg_write(c, rd, res&0xFFFF);
    pc_update(c, 2);
}

void inst_cmpi(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: CMPI\t");

    uint8_t rs = get_bits(inst, 4, 7);
    uint8_t rd = get_bits(inst, 0, 3);

    uint16_t s_data = (~sign_ext(rs, 3))+1;
    uint16_t d_data = reg_read(c, rd);
    uint32_t res = s_data+d_data;
    if(res > 0xFFFF || s_data == 0){
        c->flag_carry = getbool(0);
    }else{
        c->flag_carry = getbool(1);
    }
    c->flag_sign = getbool(flag_sign(res&0xFFFF));
    c->flag_overflow = getbool(flag_overflow(s_data, d_data, res&0xFFFF));
    c->flag_zero = getbool(flag_zero(res&0xFFFF));
    pc_update(c, 2);
}

void inst_j(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst:  J\t");

    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
    pc_update(c, imm);
}

void inst_jal(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JAL\t");

    pc_update(c, 2);

    uint16_t imm = rom_read_w(c);
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
    reg_write(c, 0, pc_read(c)+2);
    pc_update(c, imm);
}

void inst_jalr(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JALR\t");

    uint8_t rs = get_bits(inst, 4, 7);

    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
    reg_write(c, 0, pc_read(c)+2);
    pc_write(c, reg_read(c, rs));
}

void inst_jr(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JR\t");

    uint8_t rs = get_bits(inst, 4, 7);

    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
    pc_write(c, reg_read(c, rs));
}

void inst_jl(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JL\t");

    uint16_t imm = sign_ext(get_bits(inst, 0, 6)<<1, 7) ;

    if(c->flag_sign != c->flag_overflow){
        pc_update(c, imm);
    }else{
        pc_update(c, 2);
    }
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
}

void inst_jle(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JLE\t");

    uint16_t imm = sign_ext(get_bits(inst, 0, 6)<<1, 7);

    if(c->flag_sign != c->flag_overflow || c->flag_zero == 1){
        pc_update(c, imm);
    }else{
        pc_update(c, 2);
    }
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
}

void inst_je(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JE\t");

    uint16_t imm = sign_ext(get_bits(inst, 0, 6)<<1, 7);

    if(c->flag_zero == 1){
        pc_update(c, imm);
    }else{
        pc_update(c, 2);
    }
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
}

void inst_jne(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JNE\t");

    uint16_t imm = sign_ext(get_bits(inst, 0, 6)<<1, 7);

    if(c->flag_zero == 0){
        pc_update(c, imm);
    }else{
        pc_update(c, 2);
    }
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
}

void inst_jb(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JB\t");

    uint16_t imm = (sign_ext(get_bits(inst, 0, 6)<<1, 7));

    if(c->flag_carry == 1){
        pc_update(c, imm);
    }else{
        pc_update(c, 2);
    }
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
}

void inst_jbe(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: JBE\t");

    uint16_t imm = sign_ext(get_bits(inst, 0, 6)<<1, 7);

    if(c->flag_carry == 1 || c->flag_zero == 1){
        pc_update(c, imm);
    }else{
        pc_update(c, 2);
    }
    c->flag_carry = getbool(0);
    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);

}

void inst_nop(struct cpu_trit *c, uint16_t inst){
    log_printf("Inst: NOP\t");

    c->flag_sign = getbool(0);
    c->flag_overflow = getbool(0);
    c->flag_zero = getbool(0);
    c->flag_carry = getbool(0); 
    pc_update(c, 2);
}

/* ---------------------------------------------
*  Инструкции для двоичного RISC-V RV32E / RV32I
*/
const struct inst_data inst_list[] = {	
    {"0txxxxxxxxx_xxxx_xxxx", inst_lb},		//LB
    {"0txxxxxxxxx_xxxx_xxxx", inst_lh},		//LH
    {"0txxxxxxxxx_xxxx_xxxx", inst_lw},		//LW
    {"0txxxxxxxxx_xxxx_xxxx", inst_lbu},	//LBU
    {"0txxxxxxxxx_xxxx_xxxx", inst_lhu},	//LHU
    {"0txxxxxxxxx_xxxx_xxxx", inst_addi},   //ADDI
    {"0txxxxxxxxx_xxxx_xxxx", inst_slli},   //SLLI
    {"0txxxxxxxxx_xxxx_xxxx", inst_slti},   //SLTI
    {"0txxxxxxxxx_xxxx_xxxx", inst_sltiu},  //SLTIU
    {"0txxxxxxxxx_xxxx_xxxx", inst_xori},   //XORI
    {"0txxxxxxxxx_xxxx_xxxx", inst_srli},   //SRLI
    {"0txxxxxxxxx_xxxx_xxxx", inst_srai},   //SRAI
    {"0txxxxxxxxx_xxxx_xxxx", inst_ori},    //ORI
    {"0txxxxxxxxx_xxxx_xxxx", inst_andi},   //ANDI
    {"0txxxxxxxxx_xxxx_xxxx", inst_auipc},  //AUIPC
    {"0txxxxxxxxx_xxxx_xxxx", inst_sb},     //SB
    {"0txxxxxxxxx_xxxx_xxxx", inst_sh},     //SH
    {"0txxxxxxxxx_xxxx_xxxx", inst_sw},     //SW
    {"0txxxxxxxxx_xxxx_xxxx", inst_add},    //ADD
    {"0txxxxxxxxx_xxxx_xxxx", inst_sub},    //SUB
    {"0txxxxxxxxx_xxxx_xxxx", inst_sll},    //SLL
    {"0txxxxxxxxx_xxxx_xxxx", inst_slt},    //SLT
    {"0txxxxxxxxx_xxxx_xxxx", inst_sltu},   //SLTU
    {"0txxxxxxxxx_xxxx_xxxx", inst_xor},    //XOR
    {"0txxxxxxxxx_xxxx_xxxx", inst_srl},    //SRL
    {"0txxxxxxxxx_xxxx_xxxx", inst_sra},    //SRA
    {"0txxxxxxxxx_xxxx_xxxx", inst_or},     //OR
    {"0txxxxxxxxx_xxxx_xxxx", inst_and},    //AND
    {"0txxxxxxxxx_xxxx_xxxx", inst_lui},    //LUI
    {"0txxxxxxxxx_xxxx_xxxx", inst_beq},    //BEQ
    {"0txxxxxxxxx_xxxx_xxxx", inst_bne},    //BNE
    {"0txxxxxxxxx_xxxx_xxxx", inst_blt},    //BLT
    {"0txxxxxxxxx_xxxx_xxxx", inst_bge},    //BGE
    {"0txxxxxxxxx_xxxx_xxxx", inst_bltu},   //BLTU
    {"0txxxxxxxxx_xxxx_xxxx", inst_bgeu},   //BGEU
    {"0txxxxxxxxx_xxxx_xxxx", inst_jalr},   //JALR
    {"0txxxxxxxxx_xxxx_xxxx", inst_jal},    //JAL        
    {NULL, NULL} //Terminator
};
