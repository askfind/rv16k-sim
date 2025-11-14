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

void trs_pc_update(struct cpu_trit *c, tr16 offset) {	
    c->pc = add_trs(c->pc, offset);
	uint8_t nine_str[8] = {'0'};
	tr16_to_nine_string(nine_str,c->pc);
	log_printf("PC => t%s\n\r", nine_str);
}

void trs_pc_write(struct cpu_trit *c, tr16 addr) {	
	for(int i=0;i<TRIT16_SIZE;i++) {
		c->pc.t[i] = addr.t[i];
	}
}

tr16 trs_pc_read(struct cpu_trit *c) {
	tr16 r;	
	for(int i=0;i<TRIT16_SIZE;i++) {
		r.t[i] = c->pc.t[i];
	}
	return r;
}

void trs_reg_write(struct cpu_trit *c, uint8_t reg_idx, tr16 data){
    uint8_t nine_str[8] = {'0'};
    if( reg_idx > REGISTER_SIZE-1 ) {
		log_printf("Error: Reg t%d : Error reg_idx\n\r",reg_idx);
		assert(reg_idx > REGISTER_SIZE-1 && "Reg write from invalid index!");    		
		return; /* Error index. */
	}
    for(int i=0;i<TRIT16_SIZE;i++) {
		c->reg[reg_idx].t[i] = data.t[i];
	}
	tr16_to_nine_string(nine_str,c->reg[reg_idx]);
    log_printf("Reg t%d => t%s\n\r",reg_idx,nine_str);
    return; /* Ok */
}

void trs_reg_read(struct cpu_trit *c, uint8_t reg_idx, tr16 * tr ){

    uint8_t nine_str[8] = {'0'};
    if( reg_idx > REGISTER_SIZE-1 ) {
		log_printf("Error: Reg t%d : Error reg_idx\n\r",reg_idx);
		assert(reg_idx > REGISTER_SIZE-1 && "Error: Reg read from invalid index!"); 		
		return;
	}
    for(int i=0;i<TRIT16_SIZE;i++) {
		tr->t[i] = c->reg[reg_idx].t[i];
	}    
	tr16_to_nine_string(nine_str,*tr);
    log_printf("Reg t%d => t%s\n\r",reg_idx,nine_str);
    return; /*Ok */
}

void trs_rom_read_w(struct cpu_trit *c, tr16 *tr){
    
    int32_t pc;        
    pc = tr16_to_int32(c->pc);    	
	
    if( abs(pc) > (INST_ROM_SIZE/2 - 1) ) {
		log_printf("Error: ROM read from invalid address!\n\r");
		assert(abs(pc) < INST_ROM_SIZE/2 - 1 && "ROM read from invalid address!");    
		return; /* Error index. */
	}

    for(int i=0;i<TRIT8_SIZE;i++) {		
		tr->t[i] = c->inst_rom[pc].t[i]; // low
	}    
    for(int i=0;i<TRIT8_SIZE;i++) {		
		tr->t[i+TRIT8_SIZE] = c->inst_rom[pc+1].t[i]; // high;
	}     
	
    return; /* Ok */
}

void trs_mem_write_b(struct cpu_trit *c, tr16 addr, tr8 data) {
    int32_t pc;        
    pc = tr16_to_int32(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM write tr8 from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM write tr8 from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT8_SIZE;i++) {		
		c->data_ram[pc].t[i] = data.t[i];
	}    
	
    return; /* Ok */

}

void trs_mem_write_w(struct cpu_trit *c, tr16 addr, tr16 data) {
    
    int32_t pc;        
    pc = tr16_to_int32(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM read tr16 from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM read tr16 from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT8_SIZE;i++) {		
		c->data_ram[pc].t[i] = data.t[i]; // low
	}    
    for(int i=0;i<TRIT8_SIZE;i++) {		
		c->data_ram[pc+1].t[i] = data.t[i+TRIT8_SIZE]; // high;
	}     
	
    return; /* Ok */
}

void trs_mem_read_b(struct cpu_trit *c, tr16 addr, tr8 *tr) {
    int32_t pc;        
    pc = tr16_to_int32(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM read from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM read from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT8_SIZE;i++) {		
		tr->t[i] = c->data_ram[pc].t[i];
	}    
	
    return; /* Ok */
}

void trs_mem_read_w(struct cpu_trit *c, tr16 addr, tr16 *tr) {
    
    int32_t pc;        
    pc = tr16_to_int32(c->pc);    	
	
    if( abs(pc) > (DATA_RAM_SIZE/2 - 1) ) {
		log_printf("Error: RAM read tr16 from invalid address!\n\r");
		assert(abs(pc) < DATA_RAM_SIZE/2 - 1 && "RAM read tr16 from invalid address!");    
		return;
	}

    for(int i=0;i<TRIT8_SIZE;i++) {		
		tr->t[i] = c->data_ram[pc].t[i]; // low
	}    
    for(int i=0;i<TRIT8_SIZE;i++) {		
		tr->t[i+TRIT8_SIZE] = c->data_ram[pc+1].t[i]; // high;
	}     
	
    return; /* Ok */
}

//TODO Добавить реалищацию Функции режима работы троичного TRIT-RISC-V 
// 27.10.2025  
// 13.11.2025

int32_t trs_get_trits(tr16 tr, int s, int e) {
	
    trit t;
    int32_t r = 0;    	

    if( s<0 || s>15 || e<0 || e>15 || s>e ) {
		log_printf("Error: Get trits s or e from invalid position!\n\r");
		assert( s<0 || s>15 || e<0 || e>15 || s>e  && "Get trits s or e from invalid position!");    
		return;
	}	
	
	for (int i=s; i<=e; i++)
	{	
		t = get_trit(tr, i);	
		r += 3^i * (int)t;
	}    

    return r;		
}

void trs_sign_ext(tr16 tr, uint8_t sign_bit, tr16 *ret) {
	
    trit sign = get_trit(tr, sign_bit);
    
    if( sign_bit<0 || sign_bit>15 ) {
		log_printf("Error: Get trits s from invalid position!\n\r");
		assert( sign_bit<0 || sign_bit>15 && "Get trits s from invalid position!");    
		return;
	}	
	    
    for(int i=sign_bit;i<16;i++){        
        ret->t[i] = sign;
    }
    return; /* Ok */    
}

trit trs_flag_zero(tr16 res) {
	
	if ( tr16_to_int32(res) == 0 ) {
		return +1;
	}
	else {
		return -1;
	}
}

trit trs_flag_sign(tr16 res) {
    trit sng = 0;
    for(int i=0;i<TRIT16_SIZE;i++){        
         trit sng = get_trit(res,i);
         if( sng != 0 ) {
			return sng;
		 }
    }			
	return sng;
}

trit trs_flag_overflow(tr16 s1, tr16 s2, tr16 res) {
	
	//viv- old code
	trit s1_sign = get_trit(s1, 15);
    trit s2_sign = get_trit(s2, 15);
    trit res_sign = get_trit(res, 15);
    
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
	
	uint8_t buf[8];
	tr16 tr;
	tr16 tr1;
	tr16 tr2;
	tr16 tr3;	
	tr8 t8_1;
	tr16 t16_1;	
	tr16 offset; 
	tr16 addr;	
	 
	printf("[ Отладка троичных операций TRIT RISC-V ]\n\r");
	
	printf("\n\r t000: tlog(...)\n\r");
	clear_tr16(&tr);	
	tr.t[1] = 1;
	tr.t[0] = 1;	
	tlog("++ -> ", tr);
	tr.t[1] = -1;
	tr.t[0] = -1;	
	tlog("-- -> ", tr);

	printf("\n\r t001: symb_nine_form(...)\n\r");
	tr.t[1] = 1;
	tr.t[0] = 1;	
	printf("++ -> %c\n\r",symb_nine_form(tr));
	tr.t[1] = 0;
	tr.t[0] = 0;	
	printf("00 -> %c\n\r",symb_nine_form(tr));
	tr.t[1] = -1;
	tr.t[0] = -1;	
	printf("-- -> %c\n\r",symb_nine_form(tr));

	printf("\n\r t002: tr16_to_nine_string(...)\n\r");	
	tr.t[15] = -1;
	tr.t[14] =  0;
	tr.t[13] = +1;
	tr.t[12] = -1;
	tr.t[11] =  0;
	tr.t[10] = +1;
	tr.t[9] = -1;
	tr.t[8] =  0;
	tr.t[7] = +1;
	tr.t[6] = -1;
	tr.t[5] =  0;
	tr.t[4] = +1;
	tr.t[3] = -1;
	tr.t[2] =  0;
	tr.t[1] = +1;
	tr.t[0] =  0;
	tr16_to_nine_string(buf,tr);	 
	printf("tr16 -> %s\n\r",buf);
	
	printf("\n\r t003: trs_pc_update(...)\n\r");
	clear_tr16(&c->pc);
	clear_tr16(&offset);
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
	clear_tr16(&tr1);
	tlog("tr =",tr);
	trs_pc_write(&c->pc, tr);
	tr1 = trs_pc_read(&c->pc);
	tlog("tr1=",tr1);
	
	printf("\n\r t005: trs_reg_write(...) \n\r");
	clear_tr16(&tr1);
	tr1.t[2] = +1;
	tr1.t[1] =  0;
	tr1.t[0] = -1;	
	tlog("tr1 =",tr1);
	uint8_t reg_idx = 7;
	trs_reg_write(c,reg_idx,tr1);	
	reg_idx = 21;
	trs_reg_write(c,reg_idx,tr1);	
	
	printf("\n\r t006: trs_reg_read(...) \n\r");
	clear_tr16(&tr1);
	reg_idx = 7;	
	tlog("1. tr1 =",tr1);
	trs_reg_read(c, reg_idx, &tr1);
	tlog("2. tr1 =",tr1);
	
	printf("\n\r t007: trs_rom_read_w(...) \n\r");	
	clear_tr16(&c->pc);
	clear_tr16(&tr1);
	clear_tr16(&offset);

	offset.t[0] = +1;
	trs_pc_update(&c->pc, offset);
		
	tr1.t[2] = +1;
	tr1.t[1] =  0;
	tr1.t[0] = -1;	
	tlog("1. t pc =",c->pc);	
	tlog("2. t tr1 =",tr1);	
	trs_rom_read_w(c, &tr1);
	tlog("3. t pc =",c->pc);
	tlog("4. t tr1=", tr1);
	
	printf("\n\r t008: trs_mem_write_b(...), trs_mem_read_b(...) \n\r");	
	clear_tr8(&t8_1);
	clear_tr16(&addr);

	addr.t[2] = +1; 
	addr.t[1] = -1;
	addr.t[0] =  0;		

	t8_1.t[7] = -1; 
	t8_1.t[1] = +1;
	t8_1.t[0] =  0;		
	
	tlog8("t8_1 = ",t8_1);
	trs_mem_write_b(c, addr, t8_1);
	clear_tr8(&t8_1);	
	trs_mem_read_b(c, addr, &t8_1); 
	tlog8("t8_1 = ",t8_1);

	printf("\n\r t009: trs_rom_read_w(...) \n\r");	
	clear_tr16(&t16_1);
	clear_tr16(&addr);

	addr.t[2] = +1; 
	addr.t[1] = -1;
	addr.t[0] =  0;		

	t16_1.t[15] = -1; 
	t16_1.t[1] = +1;
	t16_1.t[0] =  0;		
	
	tlog("t16_1 = ",t16_1);
	trs_mem_write_w(c, addr, t16_1);
	clear_tr16(&t16_1);	
	trs_mem_read_w(c, addr, &t16_1); 
	tlog("t16_1 = ",t16_1);

	
	printf("\n\r [ END Tests. ]\n\r");
}
