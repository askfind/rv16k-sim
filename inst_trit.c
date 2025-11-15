#include <assert.h>
#include <stdio.h>
#include <stdint.h>

#include "bitpat.h"

#include "ternary.h"
#include "cpu_trit.h"
#include "inst_trit.h"
#include "log.h"

int flag_inst_trits = 1;

/** ------------------------------------------------------
 * 
 *  Реализация инструкции для троичного режима TRIT-RISC-V
 * 
 */

void trs_pc_update(struct cpu_trit *c, tr32 offset) {	
    c->pc = add_trs(c->pc, offset);
	uint8_t nine_str[16] = {'0'};
	tr32_to_nine_string(nine_str,c->pc);
	log_printf("PC => t%s\n\r", nine_str);
}

void trs_pc_write(struct cpu_trit *c, tr32 addr) {	
	for(int i=0;i<TRIT32_SIZE;i++) {
		c->pc.t[i] = addr.t[i];
	}
}

tr32 trs_pc_read(struct cpu_trit *c) {
	tr32 r;	
	for(int i=0;i<TRIT32_SIZE;i++) {
		r.t[i] = c->pc.t[i];
	}
	return r;
}

void trs_reg_write(struct cpu_trit *c, uint8_t reg_idx, tr32 data){
    uint8_t nine_str[16] = {'0'};    
    if( reg_idx > REGISTER_SIZE-1 ) {
		log_printf("Error: Reg t%d : Error reg_idx\n\r",reg_idx);
		assert(reg_idx > REGISTER_SIZE-1 && "Reg write from invalid index!");    		
		return; /* Error index. */
	}
    for(int i=0;i<TRIT32_SIZE;i++) {
		c->reg[reg_idx].t[i] = data.t[i];
	}
	tr32_to_nine_string(nine_str,c->reg[reg_idx]);
    log_printf("Reg t%d => t%s\n\r",reg_idx,nine_str);
    return; /* Ok */
}

void trs_reg_read(struct cpu_trit *c, uint8_t reg_idx, tr32 * tr ){
    uint8_t nine_str[16] = {'0'};
    if( reg_idx > REGISTER_SIZE-1 ) {
		log_printf("Error: Reg t%d : Error reg_idx\n\r",reg_idx);
		assert(reg_idx > REGISTER_SIZE-1 && "Error: Reg read from invalid index!"); 		
		return;
	}
    for(int i=0;i<TRIT32_SIZE;i++) {
		tr->t[i] = c->reg[reg_idx].t[i];
	}    
	tr32_to_nine_string(nine_str,*tr);
    log_printf("Reg t%d => t%s\n\r",reg_idx,nine_str);
    return; /*Ok */
}

void trs_rom_read_w(struct cpu_trit *c, tr32 *tr){
    
    int64_t pc;        
    pc = tr32_to_int64(c->pc);    	
	
    if( abs(pc) > (INST_ROM_SIZE/2 - 1) ) {
		log_printf("Error: ROM read from invalid address!\n\r");
		assert(abs(pc) < INST_ROM_SIZE/2 - 1 && "ROM read from invalid address!");    
		return; /* Error index. */
	}

    for(int i=0;i<TRIT16_SIZE;i++) {		
		tr->t[i] = c->inst_rom[pc].t[i]; // low
	}    
    for(int i=0;i<TRIT16_SIZE;i++) {		
		tr->t[i+TRIT8_SIZE] = c->inst_rom[pc+1].t[i]; // high;
	}     
	
    return; /* Ok */
}

void trs_mem_write_b(struct cpu_trit *c, tr32 addr, tr16 data) {
    int64_t pc;        
    pc = tr32_to_int64(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM write tr8 from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM write tr8 from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT16_SIZE;i++) {		
		c->data_ram[pc].t[i] = data.t[i];
	}    
	
    return; /* Ok */

}

void trs_mem_write_w(struct cpu_trit *c, tr32 addr, tr32 data) {
    
    int64_t pc;        
    pc = tr32_to_int64(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM read tr16 from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM read tr16 from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT16_SIZE;i++) {		
		c->data_ram[pc].t[i] = data.t[i]; // low
	}    
    for(int i=0;i<TRIT16_SIZE;i++) {		
		c->data_ram[pc+1].t[i] = data.t[i+TRIT16_SIZE]; // high;
	}     
	
    return; /* Ok */
}

void trs_mem_read_b(struct cpu_trit *c, tr32 addr, tr16 *tr) {
    int64_t pc;        
    pc = tr32_to_int64(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM read from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM read from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT16_SIZE;i++) {		
		tr->t[i] = c->data_ram[pc].t[i];
	}    
	
    return; /* Ok */
}

void trs_mem_read_w(struct cpu_trit *c, tr32 addr, tr32 *tr) {
    
    int64_t pc;        
    pc = tr32_to_int64(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM read tr16 from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM read tr16 from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT16_SIZE;i++) {		
		tr->t[i] = c->data_ram[pc].t[i]; // low
	}    
    for(int i=0;i<TRIT16_SIZE;i++) {		
		tr->t[i+TRIT16_SIZE] = c->data_ram[pc+1].t[i]; // high;
	}     
	
    return; /* Ok */
}

//TODO Добавить реалищацию Функции режима работы троичного TRIT-RISC-V 
// 27.10.2025  
// 13.11.2025

int64_t trs_get_trits(tr32 tr, int s, int e) {
	
    trit t;
    int64_t r = 0;    	

    if( s<0 || s>31 || e<0 || e>31 || s>e ) {
		log_printf("Error: Get trits s or e from invalid position!\n\r");
		assert( s<0 || s>31 || e<0 || e>31 || s>e  && "Get trits s or e from invalid position!");    
		return;
	}	
	
	for (int i=s; i<=e; i++)
	{	
		t = get_trit32(tr, i);	
		r += 3^i * (int)t;
	}    

    return r;		
}

void trs_sign_ext(tr32 tr, uint8_t sign_bit, tr32 *ret) {
	
    trit sign = get_trit32(tr, sign_bit);
    
    if( sign_bit<0 || sign_bit>31 ) {
		log_printf("Error: Get trits s from invalid position!\n\r");
		assert( sign_bit<0 || sign_bit>15 && "Get trits s from invalid position!");    
		return;
	}	
	    
    for(int i=sign_bit;i<31;i++){        
        ret->t[i] = sign;
    }
    return; /* Ok */    
}

trit trs_flag_zero(tr16 res) {
	
	if ( tr32_to_int64(res) == 0 ) {
		return +1;
	}
	else {
		return -1;
	}
}

trit trs_flag_sign(tr32 res) {
    trit sng = 0;
    for(int i=0;i<TRIT32_SIZE;i++){        
         trit sng = get_trit32(res,i);
         if( sng != 0 ) {
			return sng;
		 }
    }			
	return sng;
}

trit trs_flag_overflow(tr32 s1, tr32 s2, tr32 res) {
	
	//viv- old code
	trit s1_sign = get_trit32(s1, 31);
    trit s2_sign = get_trit32(s2, 31);
    trit res_sign = get_trit32(res, 31);
    
    // Какое условие переполнения для тритов? 
    //return ((s1_sign^s2_sign) == 0)&((s2_sign^res_sign) == 1);    
	return 0;
}

//TODO Добавить реалищацию Функции режима работы троичного TRIT-RISC-V 
// 27.10.2025  
// 13.11.2025
// 14.11.2025

/*
 * Троичные команды TRIT-RISC-V
 */  

void inst_trs_lb(struct cpu_trit *c, tr16 inst) {
	/* rd = SignExt([Address]7:0) */
	log_printf("Inst: tLB\t");	
}
void inst_trs_lh(struct cpu_trit *c, tr16 inst) {
	/* rd = SignExt([Address]15:0) */
	log_printf("Inst: tLH\t");	
}
void inst_trs_lw(struct cpu_trit *c, tr16 inst) {
	/* rd = ([Address]31:0) */
	log_printf("Inst: tLW\t");	
}
void inst_trs_lbu(struct cpu_trit *c, tr16 inst) {
	/* rd = ZeroExt([Address]7:0) */
	    log_printf("Inst: tLBU\t");	
}
void inst_trs_lhu(struct cpu_trit *c, tr16 inst) {
	/* rd = ZeroExt([Address]7:0) */
	    log_printf("Inst: tLHU\t");	
}
void inst_trs_addi(struct cpu_trit *c, tr16 inst) {
    /* rd = rs1 + SignExt(imm) */
    log_printf("Inst: tADDI\t");	
}
void inst_trs_slli(struct cpu_trit *c, tr16 inst) {
    /* rd = rs1 << uimm */
    log_printf("Inst: tSLLI\t");	
}
void inst_trs_slti(struct cpu_trit *c, tr16 inst) {
    /* rd = rs1 < SignExt(imm) */
    log_printf("Inst: tSLTI\t");	
}
void inst_trs_sltiu(struct cpu_trit *c, tr16 inst) {
    /* rd = rs1 < SignExt(imm) */
    log_printf("Inst: tSLTIU\t");	
}
void inst_trs_xori(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 xor SignExt(imm) */
	log_printf("Inst: tXORI\t");	
}
void inst_trs_srli(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 >> uimm  Shift right logical immediate */
	log_printf("Inst: tSRLI\t");	
}
void inst_trs_srai(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 >> uimm  Shift right arithmetic immediate */
	log_printf("Inst: tSRAI\t");	
}
void inst_trs_ori(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 | SignExt(imm) */
	log_printf("Inst: tORI\t");	
}
void inst_trs_andi(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 & SignExt(imm) */
	log_printf("Inst: tANDI\t");	
}
void inst_trs_auipc(struct cpu_trit *c, tr16 inst) {
	/* rd = (upimm, 12’b0) + PC */
	log_printf("Inst: tAUIPC\t");	
}
void inst_trs_sb(struct cpu_trit *c, tr16 inst) {
	/* [Address]7:0 = rs 27:0 */
	log_printf("Inst: tSB\t");	
}
void inst_trs_sh(struct cpu_trit *c, tr16 inst) {
	/* [Address]15:0 = rs2 15:0 */
	log_printf("Inst: tSH\t");	
}
void inst_trs_sw(struct cpu_trit *c, tr16 inst) {
	/* [Address]31:0 = rs2 */
	log_printf("Inst: tSW\t");	
}
void inst_trs_add(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 + rs2 */
	log_printf("Inst: tADD\t");	
}
void inst_trs_sub(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 - rs2 */
	log_printf("Inst: tSUB\t");	
}
void inst_trs_sll(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 << rs24:0 */
	log_printf("Inst: tSLL\t");	
}
void inst_trs_slt(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 < rs2 */
	log_printf("Inst: tSLT\t");	
}
void inst_trs_sltu(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 < rs2 */
	log_printf("Inst: tSLTU\t");	
}
void inst_trs_xor(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 ^ rs2 */
	log_printf("Inst: tXOR\t");	
}
void inst_trs_srl(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 >> rs24:0 */
	log_printf("Inst: tSRL\t");	
}
void inst_trs_sra(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 >>>rs24:0 */
	log_printf("Inst: tSRA\t");	
}
void inst_trs_or(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 | rs2 */
	log_printf("Inst: tOR\t");	
}
void inst_trs_and(struct cpu_trit *c, tr16 inst) {
	/* rd = rs1 & rs2 */
	log_printf("Inst: tAND\t");	
}
void inst_trs_lui(struct cpu_trit *c, tr16 inst) {
	/* rd = {upimm, 12’b0} */
	log_printf("Inst: tLUI\t");	
}
void inst_trs_beq(struct cpu_trit *c, tr16 inst) {
	/* if (rs1 == rs2) PC = BTA */
	log_printf("Inst: tBEQ\t");	
}
void inst_trs_bne(struct cpu_trit *c, tr16 inst) {
	/* if (rs1 != rs2) PC = BTA */
	log_printf("Inst: tBNE\t");	
}
void inst_trs_blt(struct cpu_trit *c, tr16 inst) {
	/* if (rs1 < rs2) PC = BTA */
	log_printf("Inst: tBLT\t");	
}
void inst_trs_bge(struct cpu_trit *c, tr16 inst) {
	/* if (rs1 ≥ rs2) PC = BTA */
	log_printf("Inst: tBGE\t");	
}
void inst_trs_bltu(struct cpu_trit *c, tr16 inst) {
	/* if (rs1 < rs2) PC = BTA */
	log_printf("Inst: tBLTU\t");	
}
void inst_trs_bgeu(struct cpu_trit *c, tr16 inst) {
	/* if (rs1 ≥ rs2) PC = BTA */
	log_printf("Inst: tBGEU\t");	
}
void inst_trs_jalr(struct cpu_trit *c, tr16 inst) {
	/* PC = rs1 + SignExt(imm) rd = PC + 4 */
	log_printf("Inst: tJALR\t");	
}
void inst_trs_jal(struct cpu_trit *c, tr16 inst) {
	/* PC = JTA rd = PC + 4 */
	log_printf("Inst: tJAL\t");	
}


/* ----------------------------------------------------
*  Инструкции для троичного TRIT-RISC-V TRV32E / TRV32I
*/
const struct inst_data trs_inst_list[] = {	
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_lb},		//tLB
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_lh},		//tLH
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_lw},		//tLW
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_lbu},	//tLBU
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_lhu},	//tLHU
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_addi},   //tADDI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_slli},   //tSLLI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_slti},   //tSLTI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sltiu},  //tSLTIU
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_xori},   //tXORI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_srli},   //tSRLI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_srai},   //tSRAI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_ori},    //tORI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_andi},   //tANDI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_auipc},  //tAUIPC
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sb},     //tSB
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sh},     //tSH
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sw},     //tSW
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_add},    //tADD
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sub},    //tSUB
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sll},    //tSLL
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_slt},    //tSLT
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sltu},   //tSLTU
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_xor},    //tXOR
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_srl},    //tSRL
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_sra},    //tSRA
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_or},     //tOR
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_and},    //tAND
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_lui},    //tLUI
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_beq},    //tBEQ
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_bne},    //tBNE
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_blt},    //tBLT
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_bge},    //tBGE
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_bltu},   //tBLTU
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_bgeu},   //tBGEU
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_jalr},   //tJALR
    {"0txxxxxxxxx_xxxx_xxxx", inst_trs_jal},    //tJAL        
    {NULL, NULL} //Terminator
};


/* Отладка троичных операций */
void trs_dbg_oper(struct cpu_trit *c) {	
	
	static uint8_t buf8[8] = {0};
	static uint8_t buf16[16] = {0};
	
	tr32 tr;
	tr32 tr1;
	tr32 tr2;
	tr32 tr3;	
	tr8 t8_1;
	tr16 t16_1;	
	tr32 t32_1;	
	tr32 offset; 
	tr32 addr;	
	 
	printf("[ Отладка троичных операций TRIT RISC-V ]\n\r");
	
	printf("\n\r t000: tlog32(...)\n\r");
	clear_tr32(&tr);	
	tr.t[1] = 1;
	tr.t[0] = 1;	
	tlog32("++ -> ", tr);
	tr.t[1] = -1;
	tr.t[0] = -1;	
	tlog32("-- -> ", tr);

	printf("\n\r t001: symb_nine_form(...)\n\r");
	t16_1.t[1] = 1;
	t16_1.t[0] = 1;	
	printf("++ -> %c\n\r",symb_nine_form(t16_1));
	t16_1.t[1] = 0;
	t16_1.t[0] = 0;	
	printf("00 -> %c\n\r",symb_nine_form(t16_1));
	t16_1.t[1] = -1;
	t16_1.t[0] = -1;	
	printf("-- -> %c\n\r",symb_nine_form(t16_1));

	printf("\n\r t002: tr16_to_nine_string(...), tr32_to_nine_string(...)\n\r");	
	clear_tr16(&t16_1);	
	t16_1.t[15] = -1;
	t16_1.t[14] =  0;
	t16_1.t[13] = +1;
	t16_1.t[12] = -1;
	t16_1.t[11] =  0;
	t16_1.t[10] = +1;
	t16_1.t[9] = -1;
	t16_1.t[8] =  0;
	t16_1.t[7] = +1;
	t16_1.t[6] = -1;
	t16_1.t[5] =  0;
	t16_1.t[4] = +1;
	t16_1.t[3] = -1;
	t16_1.t[2] =  0;
	t16_1.t[1] = +1;
	t16_1.t[0] =  0;
	tr16_to_nine_string(buf8,t16_1);	 
	printf("t16 -> ");
	buflog8(buf8);
	printf("\n\r");
	
	clear_tr32(&t32_1);	
	t32_1.t[31] = +1;
	t32_1.t[30] =  0;
	t32_1.t[29] = -1;
	t32_1.t[28] = +1;
	t32_1.t[27] =  0;
	t32_1.t[26] = -1;
	t32_1.t[25] = +1;
	t32_1.t[24] =  0;
	t32_1.t[23] = -1;
	t32_1.t[22] = +1;
	t32_1.t[21] =  0;
	t32_1.t[4] = -1;
	t32_1.t[3] = +1;
	t32_1.t[2] =  0;
	t32_1.t[1] = -1;
	t32_1.t[0] =  0;
	tr32_to_nine_string(buf16,t32_1);	 	
	printf("t32 -> ");
	buflog16(buf16);
	printf("\n\r");
	
	printf("\n\r t003: trs_pc_update(...)\n\r");
	clear_tr32(&c->pc);
	clear_tr32(&offset);
	offset.t[0] = 0;
	trs_pc_update(&c->pc, offset);
	offset.t[0] = 1;
	
	trs_pc_update(&c->pc, offset);
	tr1 = c->pc;
	trs_pc_update(&c->pc, offset);	
	trs_pc_update(&c->pc, offset);	
	trs_pc_update(&c->pc, offset);	
	trs_pc_update(&c->pc, offset);	
	trs_pc_update(&c->pc, offset);	
	
	printf("\n\r t004: trs_pc_write(...), trs_pc_read(...) \n\r");
	clear_tr32(&t32_1);
	tlog32("t32_1 =",t32_1);
	trs_pc_write(&c->pc, t32_1);
	t32_1 = trs_pc_read(&c->pc);
	tlog32("t32_1=",t32_1);
	
	printf("\n\r t005: trs_reg_write(...) \n\r");
	clear_tr32(&t32_1);
	t32_1.t[2] = +1;
	t32_1.t[1] =  0;
	t32_1.t[0] = -1;	
	tlog32("t32_1 =",t32_1);
	uint8_t reg_idx = 7;
	trs_reg_write(c,reg_idx,tr1);	
	reg_idx = 21;
	trs_reg_write(c,reg_idx,tr1);	
	
	printf("\n\r t006: trs_reg_read(...) \n\r");
	clear_tr32(&t32_1);
	reg_idx = 7;	
	tlog32("1. t32_1 =",t32_1);
	trs_reg_read(c, reg_idx, &t32_1);
	tlog32("2. t32_1 =",t32_1);
	
	printf("\n\r t007: trs_rom_read_w(...) \n\r");	
	clear_tr32(&c->pc);
	clear_tr32(&t32_1);
	clear_tr32(&offset);

	offset.t[0] = +1;
	trs_pc_update(&c->pc, offset);
		
	t32_1.t[2] = +1;
	t32_1.t[1] =  0;
	t32_1.t[0] = -1;	
	tlog32("1. t pc =",c->pc);	
	tlog32("2. t t32_1 =",t32_1);	
	trs_rom_read_w(c, &t32_1);
	tlog32("3. t pc =",c->pc);
	tlog32("4. t tr1=", t32_1);
	
	printf("\n\r t008: trs_mem_write_b(...), trs_mem_read_b(...) \n\r");	
	clear_tr16(&t16_1);
	clear_tr32(&addr);

	addr.t[2] = +1; 
	addr.t[1] = -1;
	addr.t[0] =  0;		

	t16_1.t[7] = -1; 
	t16_1.t[1] = +1;
	t16_1.t[0] =  0;		
	
	tlog16("t16_1 = ",t16_1);
	trs_mem_write_b(c, addr, t16_1);
	clear_tr16(&t16_1);	
	trs_mem_read_b(c, addr, &t16_1); 
	tlog16("t16_1 = ",t16_1);

	printf("\n\r t009: trs_rom_read_w(...) \n\r");	
	clear_tr32(&t32_1);
	clear_tr32(&addr);

	addr.t[2] = +1; 
	addr.t[1] = -1;
	addr.t[0] =  0;		

	t32_1.t[15] = -1; 
	t32_1.t[1] = +1;
	t32_1.t[0] =  0;		
	
	tlog32("t32_1 = ",t32_1);
	trs_mem_write_w(c, addr, t32_1);
	clear_tr32(&t32_1);	
	trs_mem_read_w(c, addr, &t32_1); 
	tlog32("t32_1 = ",t32_1);

	
	printf("\n\r [ END Tests. ]\n\r");
}
