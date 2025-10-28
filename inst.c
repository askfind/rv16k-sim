#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "cpu_trit.h"
#include "ternary.h"
#include "bitpat.h"
#include "inst.h"
#include "log.h"

int flag_inst_trits = 0;

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

    assert(addr < DATA_RAM_SIZE - 1 && "RAM write to invalid address!");
    
    uint8_to_trits(&c->data_ram[addr], data&0xFF);
    uint8_to_trits(&c->data_ram[addr+1], data>>8);
}

uint8_t mem_read_b(struct cpu_trit *c, uint16_t addr){
    assert(addr < DATA_RAM_SIZE && "RAM read from invalid address!");
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


/* ------------------------------------------------------
*  Реализация инструкции для троичного режима TRIT-RISC-V
*/

//TODO 27.10.2025  Добавить реалищацию Функции режима работы троичного TRIT-RISC-V 

void trs_pc_update(struct cpu_trit *c, uint16_t offset) {}
void trs_write(struct cpu_trit *c, uint16_t addr) {}
uint16_t trs_pc_read(struct cpu_trit *c) {return 0;}
void trs_reg_write(struct cpu_trit *c, uint8_t reg_idx, uint16_t data) {}
uint16_t trs_reg_read(struct cpu_trit *c, uint8_t reg_idx) {return 0;}
void trs_mem_write_b(struct cpu_trit *c, uint16_t addr, uint8_t data) {}
void trs_mem_write_w(struct cpu_trit *c, uint16_t addr, uint16_t data) {}
uint8_t trs_mem_read_b(struct cpu_trit *c, uint16_t addr) {return 0;}
uint16_t trs_mem_read_w(struct cpu_trit *c, uint16_t addr) {return 0;}
uint16_t trs_rom_read_w(struct cpu_trit *c) {return 0;}
uint16_t trs_get_bits(uint16_t t, int s, int e) {return 0;}
uint16_t trs_sign_ext(uint16_t t, uint8_t sign_bit) {return 0;}
uint8_t trs_flag_zero(uint16_t res) {return 0;}
uint8_t trs_flag_sign(uint16_t res) {return 0;}
uint8_t trs_flag_overflow(uint16_t s1, uint16_t s2, uint16_t res) {return 0;}
void inst_trs_lw(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_lwsp(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_lbu(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_lb(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_sw(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_swsp(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_sb(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_mov(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_add(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_sub(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_and(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_or(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_xor(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_lsl(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_lsr(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_asr(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_cmp(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_li(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_addi(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_cmpi(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_j(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jal(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jalr(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jr(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jl(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jle(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_je(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jne(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jb(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_jbe(struct cpu_trit *c, uint16_t inst) {}
void inst_trs_nop(struct cpu_trit *c, uint16_t inst) {}


/* -----------------------------------
*  Инструкции для двоичного RISC-V и
*  троичного TRIT-RISC-V режима работы 
*/
const struct inst_data inst_list[] = {

#if 1 //viv+ TODO add instructions RISC-V RV16 and TRIT-RISC-V RV16-TRIT
	
	/* Инструкции бинарного режима RISC-V */
    {"0b+0++_00+0_xxxx_xxxx", inst_lw},   //LW
    {"0b+0+0_xxxx_xxxx_xxxx", inst_lwsp}, //LWSP
    {"0b+0++_+0+0_xxxx_xxxx", inst_lbu},  //LBU
    {"0b+0++_+++0_xxxx_xxxx", inst_lb},   //LB
    {"0b+00+_00+0_xxxx_xxxx", inst_sw},   //SW
    {"0b+000_xxxx_xxxx_xxxx", inst_swsp}, //SWSP
    {"0b+00+_+0+0_xxxx_xxxx", inst_sb},   //SB
    {"0b+++0_0000_xxxx_xxxx", inst_mov},  //MOV
    {"0b+++0_00+0_xxxx_xxxx", inst_add},  //ADD
    {"0b+++0_00++_xxxx_xxxx", inst_sub},  //SUB
    {"0b+++0_0+00_xxxx_xxxx", inst_and},  //AND
    {"0b+++0_0+0+_xxxx_xxxx", inst_or},   //OR
    {"0b+++0_0++0_xxxx_xxxx", inst_xor},  //XOR
    {"0b+++0_+00+_xxxx_xxxx", inst_lsl},  //LSL
    {"0b+++0_+0+0_xxxx_xxxx", inst_lsr},  //LSR
    {"0b+++0_++0+_xxxx_xxxx", inst_asr},  //ASR
    {"0b++00_00++_xxxx_xxxx", inst_cmp},  //CMP
    {"0b0+++_+000_xxxx_xxxx", inst_li},   //LI
    {"0b++++_00+0_xxxx_xxxx", inst_addi}, //ADDI
    {"0b++0+_00++_xxxx_xxxx", inst_cmpi}, //CMPI
    {"0b0+0+_00+0_0000_0000", inst_j},    //J
    {"0b0+++_00++_0000_0000", inst_jal},  //JAL
    {"0b0++0_000+_xxxx_0000", inst_jalr}, //JALR
    {"0b0+00_0000_xxxx_0000", inst_jr},   //JR
    {"0b0+00_0+00_0xxx_xxxx", inst_jl},   //JL
    {"0b0+00_0+00_+xxx_xxxx", inst_jle},  //JLE
    {"0b0+00_0+0+_0xxx_xxxx", inst_je},   //JE
    {"0b0+00_0+0+_+xxx_xxxx", inst_jne},  //JNE
    {"0b0+00_0++0_0xxx_xxxx", inst_jb},   //JB
    {"0b0+00_0++0_+xxx_xxxx", inst_jbe},  //JBE
    {"0b0000_0000_0000_0000", inst_nop},  //NOP

	/* Инструкции троичного режима RISC-V */
    {"0t-0--_00-0_xxxx_xxxx", inst_trs_lw},   //tLW
    {"0t-0-0_xxxx_xxxx_xxxx", inst_trs_lwsp}, //tLWSP
    {"0t-0--_-0-0_xxxx_xxxx", inst_trs_lbu},  //tLBU
    {"0t-0--_---0_xxxx_xxxx", inst_trs_lb},   //tLB
    {"0t-00-_00-0_xxxx_xxxx", inst_trs_sw},   //tSW
    {"0t-000_xxxx_xxxx_xxxx", inst_trs_swsp}, //tSWSP
    {"0t-00-_-0-0_xxxx_xxxx", inst_trs_sb},   //tSB
    {"0t---0_0000_xxxx_xxxx", inst_trs_mov},  //tMOV
    {"0t---0_00-0_xxxx_xxxx", inst_trs_add},  //tADD
    {"0t---0_00--_xxxx_xxxx", inst_trs_sub},  //tSUB
    {"0t---0_0-00_xxxx_xxxx", inst_trs_and},  //tAND
    {"0t---0_0-0-_xxxx_xxxx", inst_trs_or},   //tOR
    {"0t---0_0--0_xxxx_xxxx", inst_trs_xor},  //tXOR
    {"0t---0_-00-_xxxx_xxxx", inst_trs_lsl},  //tLSL
    {"0t---0_-0-0_xxxx_xxxx", inst_trs_lsr},  //tLSR
    {"0t---0_--0-_xxxx_xxxx", inst_trs_asr},  //tASR
    {"0t--00_00--_xxxx_xxxx", inst_trs_cmp},  //tCMP
    {"0t0---_-000_xxxx_xxxx", inst_trs_li},   //tLI
    {"0t----_00-0_xxxx_xxxx", inst_trs_addi}, //tADDI
    {"0t--0-_00--_xxxx_xxxx", inst_trs_cmpi}, //tCMPI
    {"0t0-0-_00-0_0000_0000", inst_trs_j},    //tJ
    {"0t0---_00--_0000_0000", inst_trs_jal},  //tJAL
    {"0t0--0_000-_xxxx_0000", inst_trs_jalr}, //tJALR
    {"0t0-00_0000_xxxx_0000", inst_trs_jr},   //tJR
    {"0t0-00_0-00_0xxx_xxxx", inst_trs_jl},   //tJL
    {"0t0-00_0-00_-xxx_xxxx", inst_trs_jle},  //tJLE
    {"0t0-00_0-0-_0xxx_xxxx", inst_trs_je},   //tJE
    {"0t0-00_0-0-_-xxx_xxxx", inst_trs_jne},  //tJNE
    {"0t0-00_0--0_0xxx_xxxx", inst_trs_jb},   //tJB
    {"0t0-00_0--0_-xxx_xxxx", inst_trs_jbe},  //tJBE

#else

    {"0b1011_0010_xxxx_xxxx", inst_lw},   //LW
    {"0b1010_xxxx_xxxx_xxxx", inst_lwsp}, //LWSP
    {"0b1011_1010_xxxx_xxxx", inst_lbu},  //LBU
    {"0b1011_1110_xxxx_xxxx", inst_lb},   //LB
    {"0b1001_0010_xxxx_xxxx", inst_sw},   //SW
    {"0b1000_xxxx_xxxx_xxxx", inst_swsp}, //SWSP
    {"0b1001_1010_xxxx_xxxx", inst_sb},   //SB
    {"0b1110_0000_xxxx_xxxx", inst_mov},  //MOV
    {"0b1110_0010_xxxx_xxxx", inst_add},  //ADD
    {"0b1110_0011_xxxx_xxxx", inst_sub},  //SUB
    {"0b1110_0100_xxxx_xxxx", inst_and},  //AND
    {"0b1110_0101_xxxx_xxxx", inst_or},   //OR
    {"0b1110_0110_xxxx_xxxx", inst_xor},  //XOR
    {"0b1110_1001_xxxx_xxxx", inst_lsl},  //LSL
    {"0b1110_1010_xxxx_xxxx", inst_lsr},  //LSR
    {"0b1110_1101_xxxx_xxxx", inst_asr},  //ASR
    {"0b1100_0011_xxxx_xxxx", inst_cmp},  //CMP
    {"0b0111_1000_xxxx_xxxx", inst_li},   //LI
    {"0b1111_0010_xxxx_xxxx", inst_addi}, //ADDI
    {"0b1101_0011_xxxx_xxxx", inst_cmpi}, //CMPI
    {"0b0101_0010_0000_0000", inst_j},    //J
    {"0b0111_0011_0000_0000", inst_jal},  //JAL
    {"0b0110_0001_xxxx_0000", inst_jalr}, //JALR
    {"0b0100_0000_xxxx_0000", inst_jr},   //JR
    {"0b0100_0100_0xxx_xxxx", inst_jl},   //JL
    {"0b0100_0100_1xxx_xxxx", inst_jle},  //JLE
    {"0b0100_0101_0xxx_xxxx", inst_je},   //JE
    {"0b0100_0101_1xxx_xxxx", inst_jne},  //JNE
    {"0b0100_0110_0xxx_xxxx", inst_jb},   //JB
    {"0b0100_0110_1xxx_xxxx", inst_jbe},  //JBE
    {"0b0000_0000_0000_0000", inst_nop},  //NOP

#endif
    {NULL, NULL} //Terminator
};

